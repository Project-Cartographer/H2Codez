#pragma once
#include <ostream>
#include <functional>
#include <optional>
#include <variant>
#include <map>
#include "FastString.h"

class XMLWriter {
public:
	XMLWriter() = delete;
	XMLWriter(std::ostream& stream) : _stream(stream)
	{};
	~XMLWriter() {
		_stream.get().flush();
	}

	inline void write_self_closing(FastString name, FastString contents, std::optional<std::map<FastString, FastString>> attributes = std::nullopt) {
		start_element(name, attributes, true);
		_stream.get() << contents.get() << " >\n";
	}

	inline void write(FastString name, std::function<void(std::map<FastString, FastString>&)> setup, std::function<bool(XMLWriter&)> element) {
		std::map<FastString, FastString> attributes;
		setup(attributes);

		start_element(name, attributes, false);

		indent_level++;
		while (element(*this)) {};
		indent_level--;

		write_end_element(name);
	}

	inline void write(FastString name, std::function<bool(XMLWriter&)> element, std::optional<std::map<FastString, FastString>> attributes = std::nullopt) {
		start_element(name, attributes, false);

		indent_level++;
		while (element(*this)) {};
		indent_level--;

		write_end_element(name);
	}


private:

	inline void write_indent(int additional = 0) {
		std::fill_n(std::ostream_iterator<char>(_stream), indent_level + additional, '\t');
	}

	inline void start_element(const FastString &element, std::optional <std::map<FastString, FastString>> attributes, bool self_closing) {

		write_indent();

		_stream.get() << "<" << element.get();

		if (attributes)
			write_attributes(attributes.value());

		if (!self_closing)
			_stream.get() << ">\n";
		else
			_stream.get() << " ";
	}

	inline void write_end_element(const FastString& element) {
		write_indent();
		_stream.get() << "<\\" << element.get() << ">\n";
	}

	template <typename T>
	inline void write_attributes(T attributes) {
		for (std::pair<FastString, FastString> atpair : attributes)
			_stream.get() << " " << atpair.first.get() << " = \"" << atpair.second.get() << "\"";
	}

	int indent_level = 0;
	std::reference_wrapper<std::ostream> _stream;
};