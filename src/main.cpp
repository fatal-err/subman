#include "document.h"
#include "formats/subrip.h"
#include "utilities.h"
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct timingoptions {
  size_t gap = 0;
  int64_t shift = 0;
};

struct input {
  subman::document doc;
  timingoptions timing;
};

void transpile_timingoptions(std::vector<input> &inputs, std::string options) {
  std::vector<std::string> option_list;
  std::vector<std::string> sub_option_list;
  std::vector<std::string> option_data;
  boost::algorithm::split(option_list, options,
                          [](char c) { return c == ','; });
  size_t index = 0;
  for (auto &sub_option : option_list) { // subtitle
    sub_option_list.clear();
    boost::algorithm::trim(sub_option);
    boost::algorithm::split_regex(sub_option_list, sub_option,
                                  boost::regex("/\\s/"));
    for (auto &option : sub_option_list) { // timting options for one subtitle
      boost::algorithm::split(option_data, option, ':');
      if (option_data.size() != 2) {
        continue;
      }
      if ("shift" == option_data[0]) {
        try {
          inputs[index].timing.shift =
              boost::lexical_cast<int64_t>(option_data[1]);
        } catch (const boost::bad_lexical_cast &) {
          inputs[index].timing.shift = 0;
        }
      } else if ("gap" == option_data[0]) {
        try {
          inputs[index].timing.gap =
              boost::lexical_cast<size_t>(option_data[1]);
        } catch (const boost::bad_lexical_cast &) {
          inputs[index].timing.gap = 0;
        }
      }
    }
    index++;
  }
}

auto main(int argc, char **argv) -> int {
  namespace po = boost::program_options;
  namespace fs = boost::filesystem;
  using std::function;
  using std::map;
  using std::shared_ptr;
  using std::string;
  using std::vector;
  using subman::document;

  bool is_recursive, is_merge, is_forced, verbose;
  vector<input> inputs;
  map<string, document> outputs;

  po::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "input-files,i", po::value<vector<string>>()->multitoken(),
      "Input files")(
      "force,f",
      po::bool_switch(&is_forced)->default_value(false)->implicit_value(true),
      "Force writing on existing files.")(
      "output,o", po::value<vector<string>>()->multitoken(),
      "Output file path")("recursive,r",
                          po::bool_switch(&is_recursive)
                              ->default_value(false)
                              ->implicit_value(true)
                              ->zero_tokens(),
                          "Recursively looking for input files.")(
      "merge-method", po::value<string>()->default_value("top2bottom"),
      "The style of merge method.\n"
      "  Values:\n"
      "    top2bottom\n"
      "    bottom2top\n"
      "    left2right\n"
      "    right2left")("merge,m",
                        po::bool_switch(&is_merge)
                            ->default_value(false)
                            ->implicit_value(true)
                            ->zero_tokens(),
                        "Merge subtitles into one subtitle")(
      "styles,s", po::value<vector<string>>()->multitoken(),
      "space-separated styles for each inputs; separate each input by comma."
      "\ne.g: normal, italics red, bold #00ff00")(
      "output-format,e", po::value<string>()->default_value("auto"),
      "Output format")("verbose,v", po::bool_switch(&verbose)
                                        ->default_value(false)
                                        ->implicit_value(true)
                                        ->zero_tokens())(
      "timing,t", po::value<vector<string>>()->multitoken(),
      "space-separed timing commands for each inputs; separate "
      "each input by comma.\ne.g: gap:100ms");
  po::positional_options_description inputs_desc;
  inputs_desc.add("command", 1);
  inputs_desc.add("input-files", -1);

  po::variables_map vm;
  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(inputs_desc)
                  .run(),
              vm);
    po::notify(vm);
  } catch (std::exception const &e) {
    std::cerr << "Unknown usage of this utility. Plase use --help for more "
                 "information on how to use this program."
              << "\nError: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (!vm.count("commnad")) {
    std::cerr << "Please specify a command. Use --help for more info."
              << std::endl;
    return EXIT_FAILURE;
  }

  auto command = vm["command"].as<std::string>();

  if (!vm.count("input-files")) {
    std::cerr << "Please specify input files. Use --help for more information."
              << std::endl;
    return EXIT_FAILURE;
  }
  auto input_files = vm["input-files"].as<vector<string>>();
  std::vector<std::string> valid_input_files;
  auto output_files = !vm.count("output") ? vector<string>()
                                          : vm["output"].as<vector<string>>();
  string smm = vm["merge-method"].as<string>();
  subman::merge_method mm;
  if ("bottom2top" == smm)
    mm.direction = subman::merge_method_direction::BOTTOM_TO_TOP;
  if ("left2right" == smm)
    mm.direction = subman::merge_method_direction::LEFT_TO_RIGHT;
  if ("right2left" == smm)
    mm.direction = subman::merge_method_direction::RIGHT_TO_LEFT;
  else
    mm.direction = subman::merge_method_direction::TOP_TO_BOTTOM;

  // this function will load and add subtitles to the "documents" variable:
  function<void(string const &)> recursive_handler;
  recursive_handler = [&](string input_path) {
    // check if the path is a directory and handle file loading:
    // we will go in trouble of checking if the user has passed "recursive"
    // option to the command line
    if (is_recursive && fs::is_directory(input_path)) {
      for (auto &child : fs::directory_iterator(input_path)) {
        if (fs::is_directory(child)) {
          recursive_handler(fs::absolute(child.path().string())
                                .normalize()
                                .string()); // handle subdirectories
          continue;
        }
      }
    }
    input_path = fs::absolute(input_path).normalize().string();
    if (!fs::is_regular_file(input_path)) {
      std::cerr << "Path '" << input_path
                << "' is not a regular file or directory." << std::endl;
      return;
    }

    // it's a regular file so we push it for later to load it:
    valid_input_files.emplace_back(input_path);
  };

  // read every input files/folders:
  for (string const &input_path : input_files) {
    if (!fs::exists(input_path)) {
      std::cerr << "File '" << input_path << "' does not exists." << std::endl;
      continue;
    }
    recursive_handler(input_path);
  }

  // reading the styles
  vector<string> styles;
  if (vm.count("styles")) {
    auto data = boost::algorithm::join(vm["styles"].as<vector<string>>(), " ");
    boost::algorithm::split(styles, data, [](char c) { return c == ','; });
  }

  // reading the input files in a multithreaded environment:
  std::vector<std::thread> workers;
  std::mutex lock;
  size_t index = 0;
  for (string const &input_path : valid_input_files) {
    workers.emplace_back(
        [&](auto const &path, string style) {
          try {
            auto doc = subman::load(path);

            // applying the styles to the subtitle
            if (!style.empty()) {
              vector<string> tags;
              boost::algorithm::split_regex(tags, style, boost::regex("\\s+"));
              boost::algorithm::trim(style);
              boost::algorithm::to_lower(style);
              if (style != "normal") {
                bool bold = style == "bold" || style == "b";
                bool italic = style == "italic" || style == "i";
                bool underline = style == "underline" || style == "u";
                bool fontsize = boost::starts_with(style, boost::regex("\\d"));
                bool color = !bold && !italic && !underline && !fontsize;
                if (bold || italic || underline || fontsize || color) {
                  for (auto &sub : doc.subtitles) {
                    if (bold) {
                      sub.content.bold();
                    } else if (italic) {
                      sub.content.italic();
                    } else if (underline) {
                      sub.content.underline();
                    } else if (fontsize) {
                      sub.content.fontsize(style);
                    } else {
                      sub.content.color(style);
                    }
                  }
                }
              }
            }

            std::unique_lock<std::mutex> my_lock(lock);
            if (verbose) {
              std::cout << "Document loaded: " << path << std::endl;
              if (!style.empty()) {
                std::cout << "Style applyed: " << style << '\n' << std::endl;
              }
            }
            inputs.emplace_back(std::move(doc));
          } catch (std::exception const &e) {
            std::cerr << "Error: " << e.what() << std::endl;
          }
        },
        input_path, styles.size() > index ? styles[index++] : "");
  }
  for (auto &worker : workers) {
    worker.join();
  }
  workers.clear();

  if (inputs.empty()) {
    std::cout << "Cannot find any subtitle files. Please specify some!"
              << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("timing")) {
    transpile_timingoptions(inputs, vm["timing"].as<string>());

    // shifting stuff
    // we could just shift stuff when we were loading things; but in that
    // situation we had to do it in every single format. so we do it here, it's
    // not as performant as it should, but we'll be writing this once.
    for (auto &input : inputs) {
      if (input.timing.shift != 0) {
        decltype(input.doc.subtitles) ndoc;
        std::transform(
            std::begin(input.doc.subtitles), std::end(input.doc.subtitles),
            std::begin(input.doc.subtitles), [&](subman::subtitle &sub) {
              sub.timestamps.shift(input.timing.shift);
              return sub;
            });
      }
    }
  }
  if ("merge" == command) {

    // merge the documents into one single document:
    auto doc = inputs[0].doc;
    auto mmm = mm;
    for (auto it = std::begin(inputs) + 1; it != end(inputs); ++it) {
      mm.gap = it->timing.gap;
      doc = subman::merge(doc, it->doc, mmm);
    }
    outputs[output_files.empty() ? "" : output_files[0]] = doc;
  }

  auto format = vm["output-format"].as<string>();
  if (!outputs.empty()) {
    for (auto const &output : outputs) {
      try {
        auto &path = output.first;
        auto &doc = output.second;
        if (!path.empty()) {
          if (!is_forced && boost::filesystem::exists(path)) {
            std::cerr << "Error: File '" + path + "' already exists.";
            continue;
          }
          if (verbose) {
            std::cout << "Writing to file: " << path << std::endl;
          }
          subman::write(doc, path, format);
        } else { // printing to stdout
          subman::formats::subrip::write(doc, std::cout);
        }
      } catch (std::invalid_argument const &err) {
        std::cerr << err.what() << std::endl;
      }
    }
  } else {
    std::cerr << "There's nothing to do." << std::endl;
  }

  return EXIT_SUCCESS;
}
