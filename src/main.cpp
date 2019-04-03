#include "document.h"
#include "formats/subrip.h"
#include "utilities.h"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

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
  vector<document> inputs;
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
      "merge-method,s", po::value<string>()->default_value("top2bottom"),
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
      "italic,i", po::value<vector<bool>>()->multitoken(), "Italic font")(
      "bold,b", po::value<vector<bool>>()->multitoken(), "Bold font")(
      "color,c", po::value<vector<bool>>()->multitoken(),
      "Colors")("output-format,e", po::value<string>()->default_value("auto"),
                "Output format")("verbose,v", po::bool_switch(&verbose)
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

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

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
    //    try {
    //    if (verbose) {
    //      std::cout << "Reading file: " << input_path << std::endl;
    //    }
    valid_input_files.emplace_back(input_path);
    //    } catch (std::exception const &e) {
    //      std::cerr << "Error: " << e.what() << std::endl;
    //    }
  };

  // read every input files/folders:
  for (string const &input_path : input_files) {
    if (!fs::exists(input_path)) {
      std::cerr << "File '" << input_path << "' does not exists." << std::endl;
      continue;
    }
    recursive_handler(input_path);
  }

  // reading the input files in a multithreaded environment:
  std::vector<std::thread> workers;
  std::mutex lock;
  for (string const &input_path : valid_input_files) {
    workers.emplace_back(
        [&](auto const &path) {
          try {
            auto sub = subman::load(path);
            std::unique_lock<std::mutex> my_lock(lock);
            if (verbose) {
              std::cout << "Reading file: " << path << std::endl;
            }
            inputs.emplace_back(std::move(sub));
          } catch (std::exception const &e) {
            std::cerr << "Error: " << e.what() << std::endl;
          }
        },
        input_path);
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

  {

    // merge the documents into one single document:
    auto doc = inputs[0];
    for (auto it = std::begin(inputs) + 1; it != end(inputs); ++it) {
      doc = subman::merge(doc, *it, mm);
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
