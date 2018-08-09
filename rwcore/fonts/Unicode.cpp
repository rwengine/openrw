#include "Unicode.hpp"

#include <istream>

size_t unicode_to_utf8(unicode_t unicode, char c[4]) {
    if (unicode < 0x80) { // 7 bits
        c[0] = unicode;
        return 1;
    } else if (unicode < 0x800) { // 11 bits
        c[0] = 0xc0 | (unicode >> 6);
        c[1] = 0x80 | (unicode & 0x3f);
        return 2;
    } else if (unicode < 0x10000) { // 16 bits
        c[0] = 0xe0 | (unicode >> 12);
        c[1] = 0x80 | ((unicode >> 6) & 0x3f);
        c[2] = 0x80 | (unicode & 0x3f);
        return 3;
    } else if (unicode < 0x110000) { // 21 bits
        c[0] = 0xf0 | (unicode >> 18);
        c[1] = 0x80 | ((unicode >> 12) & 0x3f);
        c[2] = 0x80 | ((unicode >> 6) & 0x3f);
        c[3] = 0x80 | (unicode & 0x3f);
        return 4;
    } else {
        return unicode_to_utf8(UnicodeValue::UNICODE_REPLACEMENT_CHARACTER, c);
    }
}

Utf8UnicodeIterator::Utf8UnicodeIterator(std::istream &is) : m_is(&is), m_finished(false) {
    next_unicode();
}

void Utf8UnicodeIterator::next_unicode() {
    int c = m_is->get();
    if (c == EOF) {
        m_finished = true;
        return;
    }
    char cc = static_cast<char>(c);
    unicode_t unicode;
    unsigned nb_bytes;
    if ((cc & 0x80) == 0x00) {
        unicode = cc;
        nb_bytes = 0;
    } else if ((c & 0xe0) == 0xc0) {
        unicode = c & 0x1f;
        nb_bytes = 1;
    } else if ((c & 0xf0) == 0xe0) {
        unicode = c & 0x0f;
        nb_bytes = 2;
    } else if ((c & 0xf8) == 0xf0) {
        unicode = c & 0x07;
        nb_bytes = 3;
    } else {
        unicode = UnicodeValue::UNICODE_REPLACEMENT_CHARACTER;
        nb_bytes = 0;
    }
    while (nb_bytes != 0) {
        c = m_is->get();
        if (c == EOF) {
            unicode = UnicodeValue::UNICODE_REPLACEMENT_CHARACTER;
            m_finished = true;
            break;
        }
        cc = static_cast<char>(c);
        if ((c & 0xc0) != 0x80) {
            unicode = UnicodeValue::UNICODE_REPLACEMENT_CHARACTER;
            break;
        }
        unicode = (unicode << 6) | (c & 0x3f);
        --nb_bytes;
    }
    m_unicode = unicode;
}

Utf8UnicodeIterator &Utf8UnicodeIterator::operator ++() {
    next_unicode();
    return *this;
}

unicode_t Utf8UnicodeIterator::unicode() const {
    return m_unicode;
}

unicode_t Utf8UnicodeIterator::operator *() const {
    return m_unicode;
}

bool Utf8UnicodeIterator::good() const {
    return !m_finished;
}

Utf8UnicodeIteratorWrapper::Utf8UnicodeIteratorWrapper(const std::string &s)
    : iss(s) {
}

Utf8UnicodeIterator Utf8UnicodeIteratorWrapper::begin() {
    return Utf8UnicodeIterator(iss);
}

Utf8UnicodeIterator Utf8UnicodeIteratorWrapper::end() {
    return Utf8UnicodeIterator();
}

bool Utf8UnicodeIterator::operator !=(const Utf8UnicodeIterator &) {
    return good();
}
