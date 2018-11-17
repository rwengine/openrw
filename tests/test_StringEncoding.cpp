#include <boost/test/unit_test.hpp>
#include <fonts/FontMapGta3.hpp>
#include <fonts/Unicode.hpp>
#include <iostream>
#include <vector>

BOOST_TEST_DONT_PRINT_LOG_VALUE(GameString)

/**
 * All tests about changing string encodings.
 */
BOOST_AUTO_TEST_SUITE(StringEncodingTests)

BOOST_AUTO_TEST_CASE(unicode_to_char_1char) {
    char val[4];
    unicode_t u = 0x3f; /* QUESTION MARK */
    auto nb = unicode_to_utf8(u, val);

    BOOST_CHECK_EQUAL(nb, 1);

    const char ref[1] = {0x3f};
    BOOST_CHECK_EQUAL_COLLECTIONS(val, val + nb, ref, ref + nb);
}

BOOST_AUTO_TEST_CASE(unicode_to_char_2char) {
    char val[4];
    unicode_t u = 0x00e6; /* LATIN SMALL LETTER AE */
    auto nb = unicode_to_utf8(u, val);

    BOOST_CHECK_EQUAL(nb, 2);

    const char ref[2] = {char(0xc3), char(0xa6)};
    BOOST_CHECK_EQUAL_COLLECTIONS(val, val + nb, ref, ref + nb);
}

BOOST_AUTO_TEST_CASE(unicode_to_char_3char) {
    char val[4];
    unicode_t u = 0x0f45; /* TIBETAN LETTER CA */
    auto nb = unicode_to_utf8(u, val);

    BOOST_CHECK_EQUAL(nb, 3);

    const char ref[3] = {char(0xe0), char(0xbd), char(0x85)};
    BOOST_CHECK_EQUAL_COLLECTIONS(val, val + nb, ref, ref + nb);
}

BOOST_AUTO_TEST_CASE(unicode_to_char_4char) {
    char val[4];
    unicode_t u = 0x10454; /* SHAVIAN LETTER THIGH */
    auto nb = unicode_to_utf8(u, val);

    BOOST_CHECK_EQUAL(nb, 4);

    const char ref[4] = {char(0xf0), char(0x90), char(0x91), char(0x94)};
    BOOST_CHECK_EQUAL_COLLECTIONS(val, val + nb, ref, ref + nb);
}

BOOST_AUTO_TEST_CASE(unicode_to_char_illegal) {
    char val[4];
    unicode_t u = 0x124544; /* Illegal unicode */
    auto nb = unicode_to_utf8(u, val);

    BOOST_CHECK_EQUAL(nb, 3);

    const char ref[4] = {
        char(0xef), char(0xbf),
        char(0xbd)};  // utf-8 encoding of replacement character
    BOOST_CHECK_EQUAL_COLLECTIONS(val, val + nb, ref, ref + nb);
}

BOOST_AUTO_TEST_CASE(utf8_iterator_simple) {
    std::string s("Hello World", 12);
    std::istringstream iss(s);
    Utf8UnicodeIterator it(iss);

    BOOST_CHECK_EQUAL(s.size(), 12);

    for (size_t i = 0; i < s.size(); ++i) {
        BOOST_CHECK(it.good());
        BOOST_CHECK_EQUAL(it.unicode(), s[i]);
        ++it;
    }
    BOOST_CHECK(!it.good());
}

BOOST_AUTO_TEST_CASE(utf8_iterator_invalid) {
    const unsigned char s[] = {'a', 0xff, 'b', 0xff, 'c', 0x00};
    std::istringstream iss(reinterpret_cast<const char*>(s));
    Utf8UnicodeIterator it(iss);

    BOOST_CHECK_EQUAL(sizeof(s), 6);

    BOOST_CHECK(it.good());
    BOOST_CHECK_EQUAL('a', it.unicode());

    ++it;
    BOOST_CHECK(it.good());
    BOOST_CHECK_EQUAL(UNICODE_REPLACEMENT_CHARACTER, it.unicode());

    ++it;
    BOOST_CHECK(it.good());
    BOOST_CHECK_EQUAL('b', it.unicode());

    ++it;
    BOOST_CHECK(it.good());
    BOOST_CHECK_EQUAL(UNICODE_REPLACEMENT_CHARACTER, it.unicode());

    ++it;
    BOOST_CHECK(it.good());
    BOOST_CHECK_EQUAL('c', it.unicode());

    ++it;
    BOOST_CHECK(!it.good());
}

typedef struct {
    const char* utf8;
    unicode_t unicode;
} utf8_unicode_t;

const utf8_unicode_t utf_unicode_data[] = {
    {
        "\x2e", 0x2e, /* full stop*/
    },
    {
        "\x77", 0x77, /* w */
    },
    {
        "\xc3\x97", 0xd7, /* multiplication sign */
    },
    {
        "\xd8\x8c", 0x060c, /* Arabic comma */
    },
    {
        "\xe2\x9b\xb0", 0x26f0, /* mountain */
    },
    {
        "\xe2\x8a\xa8", 0x22a8, /* true */
    },
    {
        "\xf0\x9f\xa7\x9b", 0x1f9db, /* vampire */
    },
    {
        "\xf0\x9f\xa4\x9f", 0x1f91f, /* I love you hand sign */
    },
    {
        "", 0, /* sentinel */
    }};

std::string createUtf8String() {
    std::ostringstream oss;
    for (const utf8_unicode_t* uu = utf_unicode_data; uu->unicode; ++uu) {
        oss << uu->utf8;
    }
    return oss.str();
}

BOOST_AUTO_TEST_CASE(utf8_iterator_mixed) {
    std::string str = createUtf8String();
    std::istringstream iss(str);
    Utf8UnicodeIterator it(iss);

    size_t nb = 0;
    for (const utf8_unicode_t* uu = utf_unicode_data; uu->unicode; ++uu) {
        BOOST_CHECK(it.good());
        BOOST_CHECK_EQUAL(it.unicode(), uu->unicode);
        ++it;
        ++nb;
    }
    BOOST_CHECK(!it.good());
}

BOOST_AUTO_TEST_CASE(utf8_iterator_ranged_for_loop) {
    std::string str = createUtf8String();
    std::istringstream iss(str);
    Utf8UnicodeIterator it(iss);

    size_t nb = 0;
    const utf8_unicode_t* uu = utf_unicode_data;
    for (unicode_t u : Utf8UnicodeIteratorWrapper(str)) {
        BOOST_CHECK_EQUAL(u, uu->unicode);
        ++it;
        ++nb;
        ++uu;
    }
    BOOST_CHECK(!it.good());
}

BOOST_AUTO_TEST_CASE(GameStringChar_simple) {
    for (const auto& fontmap : fontmaps_gta3_font) {
        auto c = fontmap.to_GameStringChar('x');
        BOOST_CHECK_EQUAL(c, GameStringChar('x'));
        auto u = fontmap.to_unicode('x');
        BOOST_CHECK_EQUAL(u, unicode_t('x'));
    }
}

BOOST_AUTO_TEST_CASE(GameString_simple) {
    std::string s = "Hello world";
    for (const auto& fontmap : fontmaps_gta3_font) {
        auto gs = fontmap.to_GameString(s);
        BOOST_CHECK_EQUAL(s.size(), gs.length());
        for (size_t i = 0; i < s.size(); ++i) {
            BOOST_CHECK_EQUAL(gs[i], GameStringChar(s[i]));
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
