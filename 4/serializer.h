#pragma once
#include <iostream>

enum class Error {
	NoError,
	CorruptedArchive
};

class Serializer {
    static constexpr char Separator = ' ';
	std::ostream& out_;
	
	Error process(bool);

	Error process(uint64_t);
	
	template <class T>
	Error process(T&&);
	
	template<class T, class... ArgsT>
	Error process(T&&, ArgsT&&...);
public:
	explicit Serializer(std::ostream&);

	template <class T>
	Error save(T& object);

	template <class... ArgsT>
	Error operator()(ArgsT&& ...);
};

class Deserializer {
	static constexpr char Separator = ' ';
	std::istream& in_;
	
	Error process(bool&);
	
	Error process(uint64_t&);
    
	template <class T>
	Error process(T&&);
	
	template<class T, class... ArgsT>
	Error process(T&& value, ArgsT&&...);
public:
	explicit Deserializer(std::istream& in);
	
	template <class T>
	Error load(T&);
	
	template <class... ArgsT>
	Error operator()(ArgsT&&...);
};

Serializer::Serializer(std::ostream& out) : out_(out) {}

template <class T>
Error Serializer::save(T& object) {
	return object.serialize(*this);
}

template <class... ArgsT>
Error Serializer::operator()(ArgsT&&... args) {
	return process(std::forward<ArgsT>(args)...);
}

Error Serializer::process(bool val) {
	out_ << (val ? "true" : "false") << Separator;
	return Error::NoError;
}

Error Serializer::process(uint64_t val) {
	out_ << val << Separator;
	return Error::NoError;
}

template <class T>
Error Serializer::process(T&& value) {
	return Error::CorruptedArchive;
}

template<class T, class... ArgsT>
Error Serializer::process(T&& value, ArgsT&& ... args) {
	return ((process(std::forward<T>(value)) == Error::NoError) ?
        process(std::forward<ArgsT>(args)...) :
        Error::CorruptedArchive);
}




Deserializer::Deserializer(std::istream& in) : in_(in) {}

template <class T>
Error Deserializer::load(T& object) {
	return object.serialize(*this);
}

template <class... ArgsT>
Error Deserializer::operator() (ArgsT&&... args) {
	return process(std::forward<ArgsT>(args)...);
}

Error Deserializer::process(bool &value) {
	std::string in;
	in_ >> in;
	if(!in.compare("true"))
		value = true;
	else if(!in.compare("false"))
		value = false;
	else
		return Error::CorruptedArchive;
	return Error::NoError;
}

Error Deserializer::process(uint64_t &value) {
	std::string in;
	in_ >> in;
	value = 0;
	for(auto buf : in)
		if(buf >= '0' && buf <= '9') {
			value *= 10;
			value += buf - '0';
		} else
			return Error::CorruptedArchive;
	return Error::NoError;
}

template <class T>
Error Deserializer::process(T&& value) {
	return Error::CorruptedArchive;
}

template<class T, class... ArgsT>
Error Deserializer::process(T&& value, ArgsT&&... args) {
	if (process(std::forward<T>(value)) == Error::CorruptedArchive)
		return Error::CorruptedArchive;
	else
		return process(std::forward<ArgsT>(args)...);
}