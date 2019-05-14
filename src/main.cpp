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
#include "document.h"
#include "formats/subrip.h"
#include "utilities.h"

int check_arguments(
    int const argc, char const *const *const argv,
    std::map<
        std::string,
        std::function<int(boost::program_options::options_description const &,
                          boost::program_options::variables_map const &)>> const
        &actions,
    std::function<int(boost::program_options::options_description const &,
                      boost::program_options::variables_map const &)> const
        &default_action) noexcept {
  namespace po = boost::program_options;
  using std::string;
  using std::vector;

  po::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "input-files,i", po::value<vector<string>>()->multitoken(),
      "Input files")("force,f",
                     po::bool_switch()
                         ->default_value(false)
                         ->implicit_value(true)
                         ->zero_tokens(),
                     "Force writing on existing files.")(
      "output,o", po::value<vector<string>>()->multitoken(),
      "Output file path")("recursive,r",
                          po::bool_switch()
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
                        po::bool_switch()
                            ->default_value(false)
                            ->implicit_value(true)
                            ->zero_tokens(),
                        "Merge subtitles into one subtitle")(
      "styles,s", po::value<vector<string>>()->multitoken(),
      "space-separated styles for each inputs; separate each input by comma."
      "\ne.g: normal, italics red, bold #00ff00")(
      "output-format,e", po::value<string>()->default_value("auto"),
      "Output format")("verbose,v", po::bool_switch()
                                        ->default_value(false)
                                        ->implicit_value(true)
                                        ->zero_tokens());
  po::positional_options_description inputs_desc;
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

  // we need this field
  if (!vm.count("input-files")) {
    std::cerr << "Please specify input files. Use --help for more information."
              << std::endl;
    return EXIT_FAILURE;
  }

  // run the action
  auto verbose = vm["verbose"].as<bool>();
  for (auto const &action : actions) {
    if (vm.count(action.first) > 0) {
      if (verbose) std::cout << "Running " << action.first << std::endl;
      return action.second(desc, vm);
    }
  }
  return default_action(desc, vm);
}

/**
 * @brief This function will write the outputs files
 * @param vm
 * @param outputs
 */
void write(boost::program_options::variables_map const &vm,
           std::map<std::string, subman::document> const &outputs) noexcept {
  using std::string;

  auto is_forced = vm["force"].as<bool>();
  auto verbose = vm["verbose"].as<bool>();
  auto format = vm["output-format"].as<string>();

  if (!outputs.empty()) {
    for (auto const &output : outputs) {
      try {
        auto &path = output.first;
        auto &doc = output.second;
        if (!path.empty()) {
          if (!is_forced && boost::filesystem::exists(path)) {
            std::cerr << "Error: File '" + path + "' already exists."
                      << std::endl;
            continue;
          }
          if (verbose) {
            std::cout << "Writing to file: " << path << std::endl;
          }
          subman::write(doc, path, format);
        } else {  // printing to stdout
          subman::formats::subrip::write(doc, std::cout);
        }
      } catch (std::invalid_argument const &err) {
        std::cerr << err.what() << std::endl;
      }
    }
  } else {
    std::cerr << "There's nothing to do." << std::endl;
  }
}

/**
 * @brief This function will loads the input files and converts them into
 * subman::document file
 * @param vm
 * @return a vector of subman::document
 */
std::vector<subman::document> load_inputs(
    boost::program_options::variables_map const &vm) noexcept {
  using std::function;
  using std::string;
  using std::vector;
  using subman::document;
  namespace fs = boost::filesystem;

  vector<document> inputs;
  auto input_files = vm["input-files"].as<vector<string>>();
  auto is_recursive = vm["recursive"].as<bool>();
  auto verbose = vm["verbose"].as<bool>();
  std::vector<std::string> valid_input_files;

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
                                .string());  // handle subdirectories
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
  }

  return inputs;
}

/**
 * @brief print help
 * @param desc
 * @return
 */
int print_help(
    boost::program_options::options_description const &desc,
    boost::program_options::variables_map const & /* vm */) noexcept {
  std::cout << desc << std::endl;
  return EXIT_SUCCESS;
}

int merge(boost::program_options::options_description const & /* desc */,
          boost::program_options::variables_map const &vm) noexcept {
  using std::map;
  using std::string;
  using std::vector;
  using subman::document;

  map<string, document> outputs;
  auto inputs = load_inputs(vm);
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

  // merge the documents into one single document:
  auto doc = inputs[0];
  for (auto it = std::begin(inputs) + 1; it != end(inputs); ++it) {
    doc = subman::merge(doc, *it, mm);
  }
  outputs[output_files.empty() ? "" : output_files[0]] = doc;

  // write the documents
  write(vm, outputs);

  return EXIT_SUCCESS;
}

auto main(int argc, char **argv) -> int {
  namespace fs = boost::filesystem;
  using subman::document;

  return check_arguments(argc, argv, {{"help", print_help}, {"merge", merge}},
                         print_help);
}
