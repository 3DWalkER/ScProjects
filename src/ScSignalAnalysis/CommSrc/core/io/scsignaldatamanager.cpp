#include "scsignaldatamanager.h"

#include "utils/sccallbackmanager.h"
#include "scformatparser.h"

class ScConvertHelper
{
	friend class ScSignalDataManager;
public:
	/**
	 * @brief read 读取数据到缓冲区，长度由length指定
	 * @param byteSize			[in]数据字节大小
	 * @param buffer			[in]数据缓冲区
	 * @param needlen			[in]数据长度
	 * @return 实际读取的数据字节长度
	 */
	int read(const int byteSize, char* buffer, const int needlen);

private:
	ScSignalDataManager* m_pDataManager{ };	/**< 数据管理器，负责数据的读取和定位 */
	ScFormatParser* m_pParser{ };	/**< 数据格式解析器，负责验证数据头和解析数据格式 */
};

int ScConvertHelper::read(const int byteSize, char* buffer, const int needlen)
{
	return 0;
}

ScSignalDataManager::ScSignalDataManager(Sc::DataSource source, const usrid& id, int channel, int subChannel)
	: m_source(source), id(id), channel(channel), subChannel(subChannel)
{
}

ScSignalDataManager::~ScSignalDataManager()
{
}

bool ScSignalDataManager::seekSecond(int sec)
{
	if (sec < 0 || !seek(0))
		return false;

	return false;
}

int ScSignalDataManager::read(int byteSize, void* buffer, int length)
{
	const int needlen = byteSize * length;
	char* pCurrBuff = reinterpret_cast<char*>(buffer);
	if (m_pConverter)
		return m_pConverter->read(byteSize, pCurrBuff, needlen) / byteSize;
	return readRawChars(pCurrBuff, needlen) / byteSize;;
}

bool ScFileDataManager::open(Sc::ReadMode mode)
{
	isRealAtEnd = false;
	readMode = mode;
	order = 0;
	return openCurrentFile();
}

bool ScFileDataManager::isAtEnd() const
{
	if (isRealAtEnd)
		return true;

	if (!isCurrFileEof() || !nextFileName.isEmpty())
		return false;

	if (!SC_CALLBACK->nextFileNameCallback)
		return true;

	nextFileName = SC_CALLBACK->nextFileNameCallback(fileName, order + 1, Sc::DF_Unknown, hs);
	return nextFileName.isEmpty();
}

bool ScFileDataManager::seek(scuint64 offset)
{

	return false;
}

int ScFileDataManager::write(const char* data, int inlen, bool isEnd)
{
	SC_UNUSED(data); SC_UNUSED(inlen); SC_UNUSED(isEnd);
	return 0;
}

ScFileDataManager::ScFileDataManager(Sc::DataSource source, const ScString& fileName, const usrid& id, int channel, int subChannel)
	: ScSignalDataManager(source, id, channel, subChannel)
	, fileName(fileName)
{
}

int ScFileDataManager::readRawChars(char* buffer, int length)
{
	int readlen = readCurrentFile(buffer, length);
	if (readlen == length || !SC_CALLBACK->nextFileNameCallback)
		return readlen;

	if (nextFileName.isEmpty())
	{
		hs = Sc::HS_None;
		nextFileName = SC_CALLBACK->nextFileNameCallback(fileName, order + 1, Sc::DF_Unknown, hs);
		if (nextFileName.isEmpty())
		{
			isRealAtEnd = true;
			return readlen;
		}
	}

	close();
	fileName = std::move(nextFileName);
	isRealAtEnd = !openCurrentFile();
	if (isRealAtEnd)
		return readlen;

	order++;
	readlen += readCurrentFile(buffer + readlen, length - readlen);
	return readlen;
}

ScLocalDataManager::ScLocalDataManager(const ScString& fileName, const usrid& id, int channel, int subChannel)
	: ScFileDataManager(Sc::DS_Sample, fileName, id, channel, subChannel)
	, fp(nullptr)
{
}

ScLocalDataManager::~ScLocalDataManager()
{
	close();
}

bool ScLocalDataManager::isOpen() const
{
	return nullptr != fp;
}

scuint64 ScLocalDataManager::fileSize() const
{
	if (!fp)
		return 0;

#ifdef SC_OS_WIN
	struct _stat64 st;
	if (0 != _fstat64(_fileno(fp), &st))
#else
	struct stat st;
	if (0 != fstat(fileno(fp), &st))
#endif
		return 0;
	return static_cast<scuint64>(st.st_size);
}

void ScLocalDataManager::close()
{
	if (fp)
	{
		fclose(fp);
		fp = nullptr;
	}
}

bool ScLocalDataManager::openCurrentFile()
{
	if (fp)
	{
		fprintf(stderr, "%s File (%s) already open!\n", __FUNCTION__, fileName.data());
		return true;
	}

#ifdef SC_OS_WINDOWS
	return 0 == fopen_s(&fp, fileName.data(), "rb");
#else
	fp = fopen(fileName.data(), "rb");
	return fp;
#endif
}

bool ScLocalDataManager::isCurrFileEof() const
{
	return !fp || 0 != feof(fp);
}

bool ScLocalDataManager::seekCurrentFile(scuint64 offset)
{
	if (!fp)
	{
		fprintf(stderr, "%s File (%s) not open!\n", __FUNCTION__, fileName.data());
		return false;
	}

#ifdef SC_OS_WINDOWS
	return 0 == _fseeki64(fp, offset, SEEK_SET);
#else
	return 0 == fseeko(fp, offset, SEEK_SET);
#endif
}

int ScLocalDataManager::readCurrentFile(char* buffer, int length)
{
	if (!fp)
	{
		fprintf(stderr, "%s File (%s) not open!\n", __FUNCTION__, fileName.data());
		return false;
	}
	return static_cast<int>(fread(buffer, sizeof(char), length, fp));
}
