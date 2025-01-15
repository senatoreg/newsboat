#ifndef NEWSBOAT_STFLSTRING_H_
#define NEWSBOAT_STFLSTRING_H_

#include <string>

#include "libnewsboat-ffi/src/stflrichtext.rs.h" // IWYU pragma: export

namespace newsboat {

class StflRichText {
public:
	static StflRichText from_plaintext(std::string);
	static StflRichText from_quoted(std::string);

	StflRichText(const StflRichText&);
	StflRichText(StflRichText&&) = default;
	StflRichText& operator=(const StflRichText&);
	StflRichText& operator=(StflRichText&&) = default;
	~StflRichText() = default;


	void apply_style_tag(const std::string& tag, size_t start, size_t end);

	std::string plaintext() const;
	std::string stfl_quoted() const;

private:
	rust::Box<stflrichtext::bridged::StflRichText> rs_object;

	// Only constructable using the public static functions
	StflRichText(rust::Box<stflrichtext::bridged::StflRichText>&&);
};

} // namespace newsboat

#endif /* NEWSBOAT_STFLSTRING_H_ */
