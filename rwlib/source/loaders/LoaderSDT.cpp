#include <loaders/LoaderSDT.hpp>

#include <cstring>

LoaderSDT::LoaderSDT()
: m_version(GTAIIIVC)
, m_assetCount(0)
{

	}

typedef struct {
	char chunkId[4];
	uint32_t chunkSize;
	char format[4];
	struct {
		char id[4];
		uint32_t size;
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	} fmt;
	struct {
		char id[4];
		uint32_t size;
	} data;
} WaveHeader;

bool LoaderSDT::load(const std::string& filename)
{
	auto baseName = filename;
	auto sdtName = baseName + ".SDT";
	auto rawName = baseName + ".RAW";

	FILE* fp = fopen(sdtName.c_str(), "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		unsigned long fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		m_assetCount = fileSize / 20;
		m_assets.resize(m_assetCount);

		if ((m_assetCount = fread(&m_assets[0], sizeof(LoaderSDTFile), m_assetCount, fp)) != fileSize / 20) {
			m_assets.resize(m_assetCount);
			std::cout << "Error reading records in SDT archive" << std::endl;
		}

		fclose(fp);
		m_archive = rawName;
		return true;
	} else {
		return false;
	}
}

/// Get the information of a asset in the examining archive
bool LoaderSDT::findAssetInfo(size_t index, LoaderSDTFile& out)
{
	if (index < m_assets.size()) {
		out = m_assets[index];
		return true;
	}
	return false;
}

char* LoaderSDT::loadToMemory(size_t index, bool asWave)
{
	LoaderSDTFile assetInfo;
	bool found = findAssetInfo(index, assetInfo);

	if(!found) {
		std::cerr << "Asset " << std::to_string(index) << " not found!" << std::endl;
		return nullptr;
	}

	std::string rawName = m_archive;

	FILE* fp = fopen(rawName.c_str(), "rb");
	if (fp) {
		char* raw_data;
		char* sample_data;
		if (asWave) {
			raw_data = new char[sizeof(WaveHeader) + assetInfo.size];

			WaveHeader* header = reinterpret_cast<WaveHeader*>(raw_data);
			memcpy(header->chunkId, "RIFF", 4);
			header->chunkSize = sizeof(WaveHeader) - 8 + assetInfo.size;
			memcpy(header->format, "WAVE", 4);
			memcpy(header->fmt.id, "fmt ", 4);
			header->fmt.size = sizeof(WaveHeader::fmt) - 8;
			header->fmt.audioFormat = 1; // PCM
			header->fmt.numChannels = 1; // Mono
			header->fmt.sampleRate = assetInfo.sampleRate;
			header->fmt.byteRate = assetInfo.sampleRate * 2;
			header->fmt.blockAlign = 2;
			header->fmt.bitsPerSample = 16;
			memcpy(header->data.id, "data", 4);
			header->data.size = assetInfo.size;

			sample_data = raw_data + sizeof(WaveHeader);
		} else {
			raw_data = new char[assetInfo.size];
			sample_data = raw_data;
		}

		fseek(fp, assetInfo.offset, SEEK_SET);
		if (fread(sample_data, 1, assetInfo.size, fp) != assetInfo.size) {
			std::cerr << "Error reading asset " << std::to_string(index) << std::endl;
		}

		fclose(fp);
		return raw_data;
	}
	else
		return 0;
}

/// Writes the contents of assetname to filename
bool LoaderSDT::saveAsset(size_t index, const std::string& filename, bool asWave)
{
	char* raw_data = loadToMemory(index, asWave);
	if(!raw_data)
		return false;

	FILE* dumpFile = fopen(filename.c_str(), "wb");
	if(dumpFile) {
		LoaderSDTFile asset;
		if(findAssetInfo(index, asset)) {
			fwrite(raw_data, 1, asset.size + (asWave ? sizeof(WaveHeader) : 0), dumpFile);
			printf("=> SDT: Saved %zu to disk with filename %s\n", index, filename.c_str());
		}
		fclose(dumpFile);

		delete[] raw_data;
		return true;
	} else {
		delete[] raw_data;
		return false;
	}
}

/// Get the information of an asset by its index
const LoaderSDTFile &LoaderSDT::getAssetInfoByIndex(size_t index) const
{
	return m_assets[index];
}


uint32_t LoaderSDT::getAssetCount() const
{
	return m_assetCount;
}
