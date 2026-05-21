#ifndef SCSIGNALDATAMANAGER_H
#define SCSIGNALDATAMANAGER_H

#include "utils/scsaglobal.h"

/**
 * @brief The ScSignalDataManager class 数据管理器基类，定义数据读取的统一接口
 */
class ScSignalDataManager
{
public:
	virtual ~ScSignalDataManager();

	/**
	 * @brief open 打开管理器
	 * @param mode			[in]数据读取模式，默认为连续读取
	 * @return true：打开成功，false：打开失败
	 */
	virtual bool open(Sc::ReadMode mode = Sc::RM_Sequential) = 0;

	/**
	 * @brief close 关闭设备管理器
	 */
	virtual void close() = 0;

private:
	Sc::DataSource m_source;	/**< 数据来源 */

protected:
	usrid id;			/**< 数据标识 */
	int channel;		/**< 通道号 */
	int subChannel;		/**< 子通道号 */
};

#endif // SCSIGNALDATAMANAGER_H
