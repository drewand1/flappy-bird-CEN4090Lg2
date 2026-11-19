#include "textbox.h"

namespace {
	constexpr char32_t BACKSPACE = U'\b';

	constexpr bool isPrintableAscii(char32_t c) noexcept {
		return c >= U' ' && c <= U'~';
	}
}

TextBox::TextBox(cenui::Frame* frame, std::size_t maxLength, CharFilter filter)
	: mFrame(frame), mMaxLength(maxLength), mFilter(filter) {
	mValue.reserve(maxLength);
	refresh();
}

bool TextBox::handleTextEntered(const sf::Event::TextEntered& event) {
	const char32_t c = event.unicode;

	if (c == BACKSPACE) {
		if (mValue.empty())
			return false;
		mValue.pop_back();
	}
	else {
		if (!isPrintableAscii(c) || mValue.size() >= mMaxLength)
			return false;

		const char ch = static_cast<char>(c);
		if (mFilter && !mFilter(ch))
			return false;
		mValue.push_back(ch);
	}

	refresh();
	return true;
}

const std::string& TextBox::value() const noexcept {
	return mValue;
}

void TextBox::clear() {
	mValue.clear();
	refresh();
}

void TextBox::setFocused(bool focused) {
	mFocused = focused;
	refresh();
}

bool TextBox::focused() const noexcept {
	return mFocused;
}

void TextBox::refresh() {
	if (!mFrame)
		return;

	// Pad with a space when unfocused so the text doesn't shift around
	// when the caret appears/disappears (the game font is monospaced).
	mFrame->text(mValue + (mFocused ? '_' : ' '));
}
