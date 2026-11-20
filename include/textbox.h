#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SFML/Window/Event.hpp>
#include <cenui.h>
#include <cstddef>
#include <string>

class TextBox {
public:
	using CharFilter = bool (*)(char c);

	TextBox(cenui::Frame* frame, std::size_t maxLength, CharFilter filter = nullptr);

	// returns true if the text changed.
	bool handleTextEntered(const sf::Event::TextEntered& event);

	[[nodiscard]] const std::string& value() const noexcept;
	void clear();

	// caret is only shown while focused.
	void setFocused(bool focused);
	[[nodiscard]] bool focused() const noexcept;

private:
	void refresh();

	cenui::Frame* mFrame;
	std::string mValue;
	std::size_t mMaxLength;
	CharFilter mFilter;
	bool mFocused = false;
};

#endif
