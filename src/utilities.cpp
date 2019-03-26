#include "utilities.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include "formats/subrip.h"

// read from file
template <typename SubtitleType> subman::document subman::load(std::istream &in) {
  return SubtitleType::read(in);
}
subman::document subman::load(std::string const & path) {
	if (!boost::filesystem::exists(path)) {
		throw std::invalid_argument("Error: File '" + path + "' does not exits.");
	}
	std::ifstream in(path, std::ios::in);
	if (in.is_open()) {
		auto ext = boost::filesystem::extension(path);
		if ("srt" == ext) {
			return load<subman::formats::subrip>(in);
		}
		else {
			in.close();
			throw std::invalid_argument("Error: Unknown subtitle format (" + ext + ").");
		}
		in.close();
	}
	throw std::invalid_argument("Error: Cannot open '" + path + "'.");
}

// write to file
template <typename SubtitleType>
void subman::write(const subman::document &doc, std::ostream &out) {
	SubtitleType::write(doc, out);
}
void subman::write(const subman::document &doc, std::string const &path, std::string format) {
	if (boost::filesystem::exists(path)) {
		throw std::invalid_argument("Error: File '" + path + "' already exists.");
	}
	std::ofstream out(path, std::ios::out);
	if (out.is_open()) {
		auto ext = boost::filesystem::extension(path);
		if (format.empty() || "auto" == format) {
			format = ext;
		}
		if ("srt" == format) {
			subman::formats::subrip::write(doc, out);
		}
		else {
			out.close();
			throw std::invalid_argument("Error: Unknown subtitle format (" + ext + ").");
		}
		out.close();
	}
	throw std::invalid_argument("Error: Cannot open file '" + path + "'");
}
