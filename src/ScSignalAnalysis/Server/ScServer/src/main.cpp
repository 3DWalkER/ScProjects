#include "scplaybackrunnable.h"
#include "scchannelreader.h"
#include "scsignaldatamanager.h"

static void testRunnables();
static void testChannelReader();
static void testDataManager();

int main(int argc, char* argv[])
{
	testDataManager();
}

void testDataManager()
{
	ScLocalDataManager mgr(R"(I:\Sample\卫星通信\多普勒\VSR\003\CE04n003tSsBJ12r00c01-25287033800.prd)");
	mgr.open();
	size_t size = mgr.fileSize();
	std::vector<char> data(1048576);
	while (!mgr.isAtEnd())
	{
		int length = mgr.readRawChars(data.data(), data.size());
		if (length <= 0)
			break;
	}
	mgr.close();
}

void testChannelReader()
{
	ScChannelReaderPtr reader = ScChannelReader::create("0", 0, 0, false);
	if (!reader->open())
		return;

	std::thread readTh([&]() {
		FILE* fp = fopen(R"(I:\Sample\卫星通信\多普勒\VSR\003\CE04n003tSsBJ12r00c01-25287033800.prd)", "rb");
		if (!fp)
			return;

		size_t progress = 0;
		std::vector<char> data(1048576);
		while (0 == feof(fp))
		{
			size_t length = fread(data.data(), sizeof(char), data.size(), fp);
			if (0 == length)
				break;

			reader->write(data.data(), length, feof(fp));

			progress += length;
			fprintf(stdout, "读取中：%lld\r\n", progress);
		}

		fclose(fp);
	});

	std::vector<char> data(1048576);
	while (!reader->isAtEnd())
	{
		size_t length = reader->read(data.data(), data.size());
		if (length <= 0)
			break;
	}

	if (readTh.joinable())
		readTh.join();
}

void testRunnables()
{
	ScRunnableManager mgr;
	ScRunnablePtr runnable;

	do
	{
		runnable.reset(new ScPlaybackRunnable("0"));
		mgr.start(runnable);
		_sleep(10);
	} while (1);

	std::thread th([&]() {
		_sleep(1000);
		runnable->stop();
		});

	if (th.joinable())
		th.join();
}
