#ifndef _RWLIB_FONTS_UNICODE_HPP_
#define _RWLIB_FONTS_UNICODE_HPP_

#include <cstdint>
#include <iosfwd>
#include <sstream>

/**
 * unicode_t represent a unicode data point. (UTF-32)
 */
using unicode_t = char32_t;

/**
 * @brief unicode_to_utf8 Encode a unicode data point to a (non-zero terminated)
 * utf-8 string.
 * @param unicode The unicode data point to convert
 * @param c buffer to write the utf-8 data to
 * @return number of bytes written
 */
size_t unicode_to_utf8(unicode_t unicode, char c[4]);

/**
 * @brief Iterate over a utf8 string stream. Output unicode data points.
 */
class Utf8UnicodeIterator {
private:
    /**
     * @brief m_is Pointer to the utf8 stream to iterate over.
     */
    std::istream* m_is = nullptr;

    /**
     * @brief m_finished true if the stream is finished/invalid.
     */
    bool m_finished = true;

    /**
     * @brief m_unicode Current unicode point.
     */
    unicode_t m_unicode;

    /**
     * @brief next_unicode Move to the next unicode point.
     */
    void next_unicode();

public:
    /**
     * @brief Create an empty unicode iterator. The iterator is not good.
     */
    Utf8UnicodeIterator() = default;
    /**
     * @brief Create a unicode iterator that iterates over a utf8 stream.
     * @param is utf8 stream
     */
    Utf8UnicodeIterator(std::istream& is);

    /**
     * @brief operator ++ Move to the next unicode character.
     * Only call this function when the iterator is good.
     * @return this object
     */
    Utf8UnicodeIterator& operator++();
    /**
     * @brief operator != Returns true if this iterator is good
     * @return true if this iterator is good
     */
    bool operator!=(const Utf8UnicodeIterator&);

    /**
     * @brief operator * Returns the current unicode point
     * ONly call this function when the iterator is good.
     * @return unicode point
     */
    unicode_t operator*() const;

    /**
     * @brief unicode Returns the current unicode point
     * Only call this function when the iterator is good.
     * @return unicode point
     */
    unicode_t unicode() const;

    /**
     * @brief good Checks whether this stream is good.
     * A stream is good when a unicode point is available.
     * @return true if the stream is good, else false.
     */
    bool good() const;
};

/**
 * @brief This class lets you iterate over a utf8 string using for ranged loop
 */
class Utf8UnicodeIteratorWrapper {
public:
    /**
     * @brief Create a new wraper wrapper.
     * @param s The string that the utf8 to Unicode iterator should iterate over
     */
    Utf8UnicodeIteratorWrapper(const std::string& s);

    /**
     * @brief Return the iterator at the start. This one returns unicode points.
     * @return Utf8UnicodeIterator of which will read the unicode points.
     */
    Utf8UnicodeIterator begin();
    /**
     * @brief Return the unicode iterator at the end. This one is not good.
     * No unicode points can be read from this object.
     * @return Utf8UnicodeIterator Unicode iterator that is not good.
     */
    Utf8UnicodeIterator end();

private:
    /**
     * @brief iss This utf8 stream holds the wrapped string and returns bytes to
     * convert to unicode.
     */
    std::istringstream iss;
};

/**
 * Unicode data points used by OpenRW.
 */
enum UnicodeValue : unicode_t {
    UNICODE_TAB = 0x09,             /* '\t' */
    UNICODE_NEW_LINE = 0x0a,        /* '\n' */
    UNICODE_CARRIAGE_RETURN = 0x0d, /* '\r' */

    UNICODE_SPACE = 0x20,             /* ' ' */
    UNICODE_EXCLAMATION_MARK = 0x21,  /* '!' */
    UNICODE_QUOTATION_MARK = 0x22,    /* '"' */
    UNICODE_NUMBER_SIGN = 0x23,       /* '#' */
    UNICODE_DOLLAR_SIGN = 0x24,       /* '$' */
    UNICODE_PROCENT_SIGN = 0x25,      /* '%' */
    UNICODE_AMPERSAND = 0x26,         /* '&' */
    UNICODE_APOSTROPHE = 0x27,        /* ''' */
    UNICODE_LEFT_PARENTHESIS = 0x28,  /* '(' */
    UNICODE_RIGHT_PARENTHESIS = 0x29, /* ')' */
    UNICODE_ASTERISK = 0x2a,          /* '*' */
    UNICODE_PLUS_SIGN = 0x2b,         /* '+' */
    UNICODE_COMMA = 0x2c,             /* ',' */
    UNICODE_HYPHEN_MINUS = 0x2d,      /* '-' */
    UNICODE_FULL_STOP = 0x2e,         /* '.' */
    UNICODE_SOLIDUS = 0x2f,           /* '/' */

    UNICODE_DIGIT_0 = 0x30, /* '0' */
    UNICODE_DIGIT_1 = 0x31, /* '1' */
    UNICODE_DIGIT_2 = 0x32, /* '2' */
    UNICODE_DIGIT_3 = 0x33, /* '3' */
    UNICODE_DIGIT_4 = 0x34, /* '4' */
    UNICODE_DIGIT_5 = 0x35, /* '5' */
    UNICODE_DIGIT_6 = 0x36, /* '6' */
    UNICODE_DIGIT_7 = 0x37, /* '7' */
    UNICODE_DIGIT_8 = 0x38, /* '8' */
    UNICODE_DIGIT_9 = 0x39, /* '9' */

    UNICODE_COLON = 0x3a,             /* ':' */
    UNICODE_SEMICOLON = 0x3b,         /* ';' */
    UNICODE_LESS_THAN_SIGN = 0x3c,    /* '<' */
    UNICODE_EQUALS_SIGN = 0x3d,       /* '=' */
    UNICODE_GREATER_THAN_SIGN = 0x3e, /* '>' */
    UNICODE_QUESTION_MARK = 0x3f,     /* '?' */

    UNICODE_COMMERCIAL_AT = 0x40, /* '@' */
    UNICODE_CAPITAL_A = 0x41,     /* 'A' */
    UNICODE_CAPITAL_B = 0x42,     /* 'B' */
    UNICODE_CAPITAL_C = 0x43,     /* 'C' */
    UNICODE_CAPITAL_D = 0x44,     /* 'D' */
    UNICODE_CAPITAL_E = 0x45,     /* 'E' */
    UNICODE_CAPITAL_F = 0x46,     /* 'F' */
    UNICODE_CAPITAL_G = 0x47,     /* 'G' */
    UNICODE_CAPITAL_H = 0x48,     /* 'H' */
    UNICODE_CAPITAL_I = 0x49,     /* 'I' */
    UNICODE_CAPITAL_J = 0x4a,     /* 'J' */
    UNICODE_CAPITAL_K = 0x4b,     /* 'K' */
    UNICODE_CAPITAL_L = 0x4c,     /* 'L' */
    UNICODE_CAPITAL_M = 0x4d,     /* 'M' */
    UNICODE_CAPITAL_N = 0x4e,     /* 'N' */
    UNICODE_CAPITAL_O = 0x4f,     /* 'O' */
    UNICODE_CAPITAL_P = 0x50,     /* 'P' */
    UNICODE_CAPITAL_Q = 0x51,     /* 'Q' */
    UNICODE_CAPITAL_R = 0x52,     /* 'R' */
    UNICODE_CAPITAL_S = 0x53,     /* 'S' */
    UNICODE_CAPITAL_T = 0x54,     /* 'T' */
    UNICODE_CAPITAL_U = 0x55,     /* 'U' */
    UNICODE_CAPITAL_V = 0x56,     /* 'V' */
    UNICODE_CAPITAL_W = 0x57,     /* 'W' */
    UNICODE_CAPITAL_X = 0x58,     /* 'X' */
    UNICODE_CAPITAL_Y = 0x59,     /* 'Y' */
    UNICODE_CAPITAL_Z = 0x5a,     /* 'Z' */

    UNICODE_LEFT_SQUARE_BRACKET = 0x5b,  /* '[' */
    UNICODE_REVERSE_SOLIDUS = 0x5c,      /* '\' */
    UNICODE_RIGHT_SQUARE_BRACKET = 0x5d, /* ']' */
    UNICODE_CIRCUMFLEX_ACCENT = 0x5e,    /* '^' */
    UNICODE_LOW_LINE = 0x5f,             /* '_' */

    UNICODE_GRAVE_ACCENT = 0x60, /* '`' */
    UNICODE_SMALL_A = 0x61,      /* 'a' */
    UNICODE_SMALL_B = 0x62,      /* 'b' */
    UNICODE_SMALL_C = 0x63,      /* 'c' */
    UNICODE_SMALL_D = 0x64,      /* 'd' */
    UNICODE_SMALL_E = 0x65,      /* 'e' */
    UNICODE_SMALL_F = 0x66,      /* 'f' */
    UNICODE_SMALL_G = 0x67,      /* 'g' */
    UNICODE_SMALL_H = 0x68,      /* 'h' */
    UNICODE_SMALL_I = 0x69,      /* 'i' */
    UNICODE_SMALL_J = 0x6a,      /* 'j' */
    UNICODE_SMALL_K = 0x6b,      /* 'k' */
    UNICODE_SMALL_L = 0x6c,      /* 'l' */
    UNICODE_SMALL_M = 0x6d,      /* 'm' */
    UNICODE_SMALL_N = 0x6e,      /* 'n' */
    UNICODE_SMALL_O = 0x6f,      /* 'o' */
    UNICODE_SMALL_P = 0x70,      /* 'p' */
    UNICODE_SMALL_Q = 0x71,      /* 'q' */
    UNICODE_SMALL_R = 0x72,      /* 'r' */
    UNICODE_SMALL_S = 0x73,      /* 's' */
    UNICODE_SMALL_T = 0x74,      /* 't' */
    UNICODE_SMALL_U = 0x75,      /* 'u' */
    UNICODE_SMALL_V = 0x76,      /* 'v' */
    UNICODE_SMALL_W = 0x77,      /* 'w' */
    UNICODE_SMALL_X = 0x78,      /* 'x' */
    UNICODE_SMALL_Y = 0x79,      /* 'y' */
    UNICODE_SMALL_Z = 0x7a,      /* 'z' */

    UNICODE_LEFT_CURLY_BRACKET = 0x7b,  /* '{' */
    UNICODE_VERTICAL_LINE = 0x7c,       /* '|' */
    UNICODE_RIGHT_CURLY_BRACKET = 0x7d, /* '}' */
    UNICODE_TILDE = 0x7e,               /* '~' */

    UNICODE_INVERTED_EXCLAMATION_MARK = 0xa1, /* '¡' */
    UNICODE_COPYRIGHT_SIGN = 0xa9,            /* '©' */
    UNICODE_REGISTERED_SIGN = 0xae,           /* '®' */
    UNICODE_DEGREES = 0xb0,                   /* '°' */
    UNICODE_ACUTE_ACCENT = 0xb4,              /* '´' */
    UNICODE_INVERTED_QUESTION_MARK = 0xbf,    /* '¿' */

    UNICODE_CAPITAL_A_GRAVE = 0xc0,      /* 'À' */
    UNICODE_CAPITAL_A_ACUTE = 0xc1,      /* 'Á' */
    UNICODE_CAPITAL_A_CIRCUMFLEX = 0xc2, /* 'Â' */
    UNICODE_CAPITAL_A_DIARESIS = 0xc4,   /* 'Ä' */
    UNICODE_CAPITAL_AE = 0xc6,           /* 'Æ' */
    UNICODE_CAPITAL_C_CEDILLA = 0xc7,    /* 'Ç' */
    UNICODE_CAPITAL_E_GRAVE = 0xc8,      /* 'È' */
    UNICODE_CAPITAL_E_ACUTE = 0xc9,      /* 'É' */
    UNICODE_CAPITAL_E_CIRCUMFLEX = 0xca, /* 'Ê' */
    UNICODE_CAPITAL_E_DIARESIS = 0xcb,   /* 'Ë' */
    UNICODE_CAPITAL_I_GRAVE = 0xcc,      /* 'Ì' */
    UNICODE_CAPITAL_I_ACUTE = 0xcd,      /* 'Í' */
    UNICODE_CAPITAL_I_CIRCUMFLEX = 0xce, /* 'Î' */
    UNICODE_CAPITAL_I_DIARESIS = 0xcf,   /* 'Ï' */
    UNICODE_CAPITAL_N_TILDE = 0xd1,      /* 'Ñ' */
    UNICODE_CAPITAL_O_GRAVE = 0xd2,      /* 'Ò' */
    UNICODE_CAPITAL_O_ACUTE = 0xd3,      /* 'Ó' */
    UNICODE_CAPITAL_O_CIRCUMFLEX = 0xd4, /* 'Ô' */
    UNICODE_CAPITAL_O_DIARESIS = 0xd6,   /* 'Ö' */
    UNICODE_MULTIPLICATION_SIGN = 0xd7,  /* '×' */
    UNICODE_CAPITAL_U_GRAVE = 0xd9,      /* 'Ù' */
    UNICODE_CAPITAL_U_ACUTE = 0xda,      /* 'Ú' */
    UNICODE_CAPITAL_U_CIRCUMFLEX = 0xdb, /* 'Û' */
    UNICODE_CAPITAL_U_DIARESIS = 0xdc,   /* 'Ü' */
    UNICODE_SMALL_SHARP_S = 0xdf,        /* 'ß' */

    UNICODE_SMALL_A_GRAVE = 0xe0,      /* 'à' */
    UNICODE_SMALL_A_ACUTE = 0xe1,      /* 'á' */
    UNICODE_SMALL_A_CIRCUMFLEX = 0xe2, /* 'â' */
    UNICODE_SMALL_A_DIARESIS = 0xe4,   /* 'ä' */
    UNICODE_SMALL_AE = 0xe6,           /* 'æ' */
    UNICODE_SMALL_C_CEDILLA = 0xe7,    /* 'ç' */
    UNICODE_SMALL_E_GRAVE = 0xe8,      /* 'è' */
    UNICODE_SMALL_E_ACUTE = 0xe9,      /* 'é' */
    UNICODE_SMALL_E_CIRCUMFLEX = 0xea, /* 'ê' */
    UNICODE_SMALL_E_DIARESIS = 0xeb,   /* 'ë' */
    UNICODE_SMALL_I_GRAVE = 0xec,      /* 'ì' */
    UNICODE_SMALL_I_ACUTE = 0xed,      /* 'í' */
    UNICODE_SMALL_I_CIRCUMFLEX = 0xee, /* 'î' */
    UNICODE_SMALL_I_DIARESIS = 0xef,   /* 'ï' */
    UNICODE_SMALL_N_TILDE = 0xf1,      /* 'ñ' */
    UNICODE_SMALL_O_GRAVE = 0xf2,      /* 'ò' */
    UNICODE_SMALL_O_ACUTE = 0xf3,      /* 'ó' */
    UNICODE_SMALL_O_CIRCUMFLEX = 0xf4, /* 'ô' */
    UNICODE_SMALL_O_DIARESIS = 0xf6,   /* 'ö' */
    UNICODE_SMALL_U_GRAVE = 0xf9,      /* 'ù' */
    UNICODE_SMALL_U_ACUTE = 0xfa,      /* 'ú' */
    UNICODE_SMALL_U_CIRCUMFLEX = 0xfb, /* 'û' */
    UNICODE_SMALL_U_DIARESIS = 0xfc,   /* 'ü' */

    UNICODE_NUMERO_SIGN = 0x2116,                  /* '№' */
    UNICODE_TRADE_MARK = 0x2122,                   /* '™' */
    UNICODE_INCREMENT = 0x2206,                    /* '∆' */
    UNICODE_BLACK_UP_POINTING_TRIANGLE = 0x25b2,   /* '▲' */
    UNICODE_BLACK_RIGHT_POINTING_POINTER = 0x25ba, /* '►' */
    UNICODE_BLACK_DOWN_POINTING_POINTER = 0x25bc,  /* '▼' */
    UNICODE_BLACK_LEFT_POINTING_POINTER = 0x25c4,  /* '◄' */
    UNICODE_WHITE_CIRCLE = 0x25cb,                 /* '○' */
    UNICODE_BLACK_STAR = 0x2605,                   /* '★' */
    UNICODE_BLACK_HEART_SUIT = 0x2665,             /* '♥' */
    UNICODE_CROSS_MARK = 0x274c,                   /* '❌' */
    UNICODE_REPLACEMENT_CHARACTER = 0xfffd,        /* '�' */

    UNICODE_SHIELD = 0x1f6e1, /* '🛡' */
};

#endif
