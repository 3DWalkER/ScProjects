#include "scplaybackrunnable.h"

int main(int argc, char* argv[])
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