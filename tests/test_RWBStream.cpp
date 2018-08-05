#include <boost/test/unit_test.hpp>
#include <loaders/RWBinaryStream.hpp>
#include <platform/FileHandle.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(RWBStreamTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(iterate_stream_test) {
    {
        auto d = Global::get().e->data->index.openFile("landstal.dff");

        RWBStream stream(d.data.get(), d.length);

        RWBStream::ChunkID id = stream.getNextChunk();

        BOOST_REQUIRE_EQUAL(id, 0x0010);

        auto inner = stream.getInnerStream();

        auto inner1 = inner.getNextChunk();

        BOOST_REQUIRE_EQUAL(inner1, 0x0001);

        auto innerCursor = inner.getCursor();

        // This is a value inside in the Clump's struct header section.
        BOOST_CHECK_EQUAL(*reinterpret_cast<std::uint32_t*>(innerCursor), 0x10);
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
