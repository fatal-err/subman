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
#include <numeric>
#include <string>
#include <thread>
#include <vector>

struct timing_options {
  size_t gap = 0;
  int64_t shift = 0;
};

/**
 * @brief transpile "--timing" values into timing_options struct
 * @param options
 * @return a vector of timing_options
 */
std::vector<timing_options>
transpile_timing_options(std::vector<std::string> const& options) {
  std::vector<std::string> option_list;
  std::vector<std::string> sub_option_list;
  std::vector<std::string> option_data;
  std::string soptions = boost::algorithm::join(options, " ");
  boost::algorithm::split(
      option_list, soptions, [](char c) { return c == ','; });
  size_t index = 0;
  std::vector<timing_options> timings;
  for (auto& sub_option : option_list) { // subtitle
    sub_option_list.clear();
    boost::algorithm::trim(sub_option);
    boost::algorithm::split_regex(
        sub_option_list, sub_option, boost::regex("/\\s/"));
    for (auto& option : sub_option_list) { // timting options for one subtitle
      boost::algorithm::split(
          option_data, option, [](char c) { return c == ':'; });
      if (option_data.size() != 2) {
        continue;
      }
      if ("shift" == option_data[0]) {
        while (timings.size() <= index)
          timings.emplace_back();
        try {
          timings[index].shift = boost::lexical_cast<int64_t>(option_data[1]);
        } catch (const boost::bad_lexical_cast&) {
          timings[index].shift = 0;
        }
      } else if ("gap" == option_data[0]) {
        while (timings.size() <= index)
          timings.emplace_back();
        try {
          timings[index].gap = boost::lexical_cast<size_t>(option_data[1]);
        } catch (const boost::bad_lexical_cast&) {
          timings[index].gap = 0;
        }
      }
    }
    index++;
  }
  return timings;
}

int check_arguments(
    int const argc,
    char const* const* const argv,
    std::map<
        std::string,
        std::function<int(boost::program_options::options_description const&,
                          boost::program_options::variables_map const&)>> const&
        actions,
    std::function<int(boost::program_options::options_description const&,
                      boost::program_options::variables_map const&)> const&
        default_action) noexcept {
  namespace po = boost::program_options;
  using std::string;
  using std::vector;

  // join the keys in action so we can print them as a string
  std::string possible_values =
      std::accumulate(std::next(actions.cbegin()),
                      actions.cend(),
                      actions.cbegin()->first,
                      [](std::string all, auto const& a) {
                        return std::move(all) + ", " + a.first;
                      });

  po::options_description desc("SubMan (Subtitle Manager)");
  desc.add_options()("help,h", "Show this help page.")(
      "input-files,i",
      po::value<vector<string>>()->multitoken(),
      "Input files")("force,f",
                     po::bool_switch()
                         ->default_value(false)
                         ->implicit_value(true)
                         ->zero_tokens(),
                     "Force writing on existing files.")(
      "output,o",
      po::value<vector<string>>()->multitoken(),
      "Output file path")("recursive,r",
                          po::bool_switch()
                              ->default_value(false)
                              ->implicit_value(true)
                              ->zero_tokens(),
                          "Recursively looking for input files.")(
      "merge-method",
      po::value<string>()->default_value("top2bottom"),
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
      "styles,s",
      po::value<vector<string>>()->multitoken(),
      "space-separated styles for each inputs; separate each input by comma."
      "\ne.g: normal, italic red, bold #00ff00")(
      "output-format,e",
      po::value<string>()->default_value("auto"),
      "Output format")(
      "style",
      po::bool_switch()
          ->default_value(false)
          ->implicit_value(true)
          ->zero_tokens(),
      "It's command that will let you style the input file with the help of "
      "--style or change the timings with --timing and etc.")(
      "verbose,v",
      po::bool_switch()
          ->default_value(false)
          ->implicit_value(true)
          ->zero_tokens())(
      "timing,t",
      po::value<vector<string>>()->multitoken(),
      "space-separed timing commands for each inputs; separate "
      "each input by comma.\ne.g: gap:100ms")("override",
                                              po::bool_switch()
                                                  ->default_value(false)
                                                  ->implicit_value(true)
                                                  ->zero_tokens())(
      "command",
      po::value<std::string>()->default_value("help"),
      ("the command. possible values: " + possible_values).c_str())
          ("contains,c", po::value<std::vector<std::string>>()->multitoken(), "Search for subtitles that contain the specified values.")
          ("matches,m", po::value<std::vector<std::string>>()->multitoken(),
                  "Filter the results to those subtitles that match the specified values.")
          ("regex", po::value<std::vector<std::string>>()->multitoken(),
                  "Filter the results bases on those subtitles that match the specified regular expressions.");
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
  } catch (std::exception const& e) {
    std::cerr << "Unknown usage of this utility. Plase use --help for more "
                 "information on how to use this program."
              << "\nError: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // no command
  if (!vm.count("command")) {
    std::cerr << "Please specify a command. Use --help for more info."
              << std::endl;
    return EXIT_FAILURE;
  }

  // run the action
  auto verbose = vm["verbose"].as<bool>();
  auto command = vm["command"].as<string>();
  auto action = actions.find(command);
  if (action != std::end(actions)) {
    if (verbose)
      std::cout << "Running " << action->first << std::endl;
    return action->second(desc, vm);
  }
  if (verbose)
    std::cout << "The specified command (" << command
              << ") is undefined; running default operation." << std::endl;
  return default_action(desc, vm);
}

/**
 * @brief This function will write the outputs files
 * @param vm
 * @param outputs
 */
void write(boost::program_options::variables_map const& vm,
           std::map<std::string, subman::document> const& outputs) noexcept {
  using std::string;

  auto is_forced = vm["force"].as<bool>();
  auto override_files = vm["override"].as<bool>();
  auto verbose = vm["verbose"].as<bool>();
  auto format = vm["output-format"].as<string>();
  auto input_files = vm["input-files"].as<std::vector<string>>();

  if (!outputs.empty()) {
    auto it = input_files.cbegin();
    for (auto const& output : outputs) {
      try {
        auto& path = output.first;
        auto& doc = output.second;
        if (!path.empty() && path != "--") {
          if ((!is_forced && boost::filesystem::exists(path)) ||
              (!override_files && *it == path)) {
            std::cerr << "Error: File '" + path + "' already exists."
                      << std::endl;
            it++;
            continue;
          }
          if (verbose) {
            std::cout << "Writing to file: " << path << std::endl;
          }
          subman::write(doc, path, format);
        } else { // printing to stdout
          subman::formats::subrip::write(doc, std::cout);
          std::cout << std::flush;
        }
      } catch (std::invalid_argument const& err) {
        if (verbose)
          std::cerr << err.what() << std::endl;
      }
      it++;
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
std::vector<subman::document>
load_inputs(boost::program_options::variables_map const& vm) noexcept {
  using std::function;
  using std::string;
  using std::vector;
  using subman::document;
  namespace fs = boost::filesystem;

  vector<subman::document> inputs;
  auto verbose = vm["verbose"].as<bool>();

  // we need this field
  if (!vm.count("input-files")) {
    if (verbose) {
      std::cerr
          << "Please specify input files. Use --help for more information."
          << std::endl;
    }
    return inputs;
  }

  auto input_files = vm["input-files"].as<vector<string>>();
  auto is_recursive = vm["recursive"].as<bool>();
  std::vector<std::string> valid_input_files;

  // this function will load and add subtitles to the "documents" variable:
  function<void(string const&)> recursive_handler;
  recursive_handler = [&](string input_path) {
    // check if the path is a directory and handle file loading:
    // we will go in trouble of checking if the user has passed "recursive"
    // option to the command line
    if (is_recursive && fs::is_directory(input_path)) {
      for (auto& child : fs::directory_iterator(input_path)) {
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
  for (string const& input_path : input_files) {
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

  // handle --timing options and apply changes
  vector<timing_options> timings;
  if (vm.count("timing")) {
    timings = transpile_timing_options(vm["timing"].as<vector<string>>());
  }

  // reading the input files in a multithreaded environment:
  std::vector<std::thread> workers;
  std::mutex lock;
  size_t index = 0;
  for (string const& input_path : valid_input_files) {
    workers.emplace_back(
        [&](auto const& path, string style, timing_options const& timing) {
          try {
            auto doc = subman::load(path);

            // applying the styles to the subtitle
            if (!style.empty()) {
              vector<string> tags;
              boost::algorithm::split_regex(tags, style, boost::regex("\\s+"));
              boost::algorithm::trim(style);
              boost::algorithm::to_lower(style);
              bool bold = false;
              bool italic = false;
              bool underline = false;
              string fontsize = "";
              string color = "";
              for (auto const& tag : tags) {
                if (tag == "normal") {
                  bold = false;
                  underline = false;
                  italic = false;
                } else if (tag == "b" || tag == "bold" || tag == "strong")
                  bold = true;
                else if (tag == "u" || tag == "underline" ||
                         tag == "underlined")
                  underline = true;
                else if (tag == "i" || tag == "italic" || tag == "italics")
                  italic = true;
                else if (boost::starts_with(tag, boost::regex("\\d")))
                  fontsize = tag;
                else
                  color = tag;
              }
              for (auto& sub : doc.subtitles) {
                if (bold)
                  sub.content.bold();
                if (italic)
                  sub.content.italic();
                if (underline)
                  sub.content.underline();
                if (!fontsize.empty())
                  sub.content.fontsize(fontsize);
                if (!color.empty())
                  sub.content.color(color);
              }
            }

            if (timing.gap != 0)
              doc.gap(timing.gap);
            if (timing.shift != 0)
              doc.shift(timing.shift);

            std::unique_lock<std::mutex> my_lock(lock);
            if (verbose) {
              std::cout << "Document loaded: " << path << std::endl;
              if (!style.empty()) {
                std::cout << "Style applyed: " << style << '\n' << std::endl;
              }
            }
            inputs.emplace_back(std::move(doc));

          } catch (std::exception const& e) {
            std::cerr << "Error: " << e.what() << std::endl;
          }
        },
        input_path,
        (styles.size() > index ? styles[index] : ""),
        (timings.size() > index ? timings[index] : timing_options{0, 0}));
    index++;
  }
  for (auto& worker : workers)
    worker.join();
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
int print_help(boost::program_options::options_description const& desc,
               boost::program_options::variables_map const& /* vm */) noexcept {
  std::cout << "Usage: subman command [input-files...] [args...]\n"
            << desc << std::endl;
  return EXIT_SUCCESS;
}


subman::merge_method get_merge_method(boost::program_options::variables_map const &vm) noexcept  {
  auto smm = vm["merge-method"].as<std::string>();
  subman::merge_method mm;

  if ("bottom2top" == smm)
    mm.direction = subman::merge_method_direction::BOTTOM_TO_TOP;
  if ("left2right" == smm)
    mm.direction = subman::merge_method_direction::LEFT_TO_RIGHT;
  if ("right2left" == smm)
    mm.direction = subman::merge_method_direction::RIGHT_TO_LEFT;
  else
    mm.direction = subman::merge_method_direction::TOP_TO_BOTTOM;

  return mm;
}

int search(boost::program_options::options_description const&,
        boost::program_options::variables_map const& vm) noexcept {
    auto contains = vm.count("contains") ? vm["contains"].as<std::vector<std::string>>() : std::vector<std::string>();
    auto matches = vm.count("matches") ? vm["matches"].as<std::vector<std::string>>() : std::vector<std::string>();
    auto regexes = vm.count("regex") ? vm["regex"].as<std::vector<std::string>>() : std::vector<std::string>();

    auto inputs = load_inputs(vm);
    auto output_files = vm.count("output") ? vm["output"].as<std::vector<std::string>>() : std::vector<std::string>();
    std::map<std::string, subman::document> outputs;
    auto mm = get_merge_method(vm);

    auto output_files_it = std::begin(output_files);
    for (auto const &input : inputs) {
        auto filtered = input; // copy the input
        for (auto& m : matches)
            filtered = filtered.matches(m);
        for (auto &c : contains)
            filtered = filtered.contains(c);
        for (auto &r : regexes)
            filtered = filtered.regex(r);

        auto output_file = output_files_it == std::end(output_files) ? "" : *output_files_it;
        if (outputs.find(output_file) == outputs.cend())
          outputs[output_file] = std::move(filtered);
        else
          outputs[output_file] = subman::merge(outputs[output_file], filtered, mm);
    }

    // write to the outputs
    write(vm, outputs);

    return EXIT_SUCCESS;
}

/**
 * @brief merge two or more subtitles into one single subtitle
 * @param vm
 * @return
 */
int merge(boost::program_options::options_description const& /* desc */,
          boost::program_options::variables_map const& vm) noexcept {
  using std::map;
  using std::string;
  using std::vector;
  using subman::document;

  map<string, document> outputs;
  auto inputs = load_inputs(vm);
  if (inputs.empty()) {
    std::cerr << "There's no input file to work on. Please specify some."
              << std::endl;
    return EXIT_FAILURE;
  }
  auto output_files = vm.count("output") ? vm["output"].as<vector<string>>() : vector<string>();
  auto mm = get_merge_method(vm);

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

int style(boost::program_options::options_description const& /* desc */,
          boost::program_options::variables_map const& vm) noexcept {
  using std::string;
  using std::vector;

  auto input_files = vm["input-files"].as<vector<string>>();
  bool override_files = vm["override"].as<bool>();
  auto output_files = !vm.count("output")
                          ? (override_files ? input_files : vector<string>())
                          : vm["output"].as<vector<string>>();
  auto inputs = load_inputs(vm);

  if (inputs.empty()) {
    std::cerr << "There's no subtitle to give style. Input one." << std::endl;
    return EXIT_FAILURE;
  }

  // making sure the output_files.size == input.size
  if (output_files.size() > inputs.size()) {
    output_files.erase(output_files.begin() + static_cast<long>(inputs.size()));
  } else if (output_files.size() < inputs.size()) {
    if (override_files) {
      for (auto it =
               input_files.begin() + static_cast<long>(output_files.size());
           it != std::end(input_files);
           it++) {
        output_files.emplace_back(*it);
      }
    } else {
      auto size = input_files.size() - output_files.size();
      output_files.insert(output_files.end(), size, "--");
    }
  }

  // converting inputs to outputs. the styles have already been done in the
  // load_inputs function
  std::map<string, subman::document> outputs;
  std::transform(inputs.begin(),
                 inputs.end(),
                 std::inserter(outputs, outputs.end()),
                 [ip = output_files.begin()](auto& input) mutable {
                   return std::make_pair(std::move(*(ip++)), std::move(input));
                 });

  // write into their outputs
  write(vm, outputs);

  return EXIT_SUCCESS;
}

/**
 * @brief append the input files into one single document
 * @param vm
 * @return
 */
int append(boost::program_options::options_description const& /* desc */,
           boost::program_options::variables_map const& vm) noexcept {
  auto inputs = load_inputs(vm);
  auto output_files = vm.count("output")
                          ? vm["output"].as<std::vector<std::string>>()
                          : std::vector<std::string>();
  auto output_file = output_files.empty() ? "--" : output_files.at(0);
  if (inputs.empty()) {
    std::cout << "We need some subtitles to work on. Specify some."
              << std::endl;
    return EXIT_FAILURE;
  }

  auto output = std::move(inputs.at(0));
  for (auto it = inputs.begin() + 1; it != inputs.end(); it++) {
    it->shift(output.subtitles.rbegin()->timestamps.to);
    output.subtitles.insert(std::make_move_iterator(it->subtitles.begin()),
                            std::make_move_iterator(it->subtitles.end()));
  }

  std::map<std::string, subman::document> outputs;
  outputs[std::move(output_file)] = std::move(output);
  write(vm, outputs);

  return EXIT_SUCCESS;
}

auto main(int argc, char** argv) -> int {
  return check_arguments(argc,
                         argv,
                         {{"help", print_help},
                          {"merge", merge},
                          {"style", style},
                          {"append", append},
                          {"search", search}},
                         print_help);
}
