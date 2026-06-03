#ifndef SCCALLBACKMANAGER_H
#define SCCALLBACKMANAGER_H

#include "scsaglobal.h"
#include <functional>

/**
 * @brief The ScGraphicCallback type 接受图形数据的回调函数类型
 * @param dataType		[in]数据类型
 * @param bitDepth		[in]量化比特数
 * @param data			[in]图形数据
 * @param length		[in]数据长度
 */
using ScGraphicCallback = std::function<void(Sc::DataType dataType, Sc::BitDepth bitDepth, const char* data, int length)>;

/**
 * @brief The ScProcessStateCallback type 处理进度回调函数
 * @param type			[in]处理类型
 * @param isStart		[in]是否为启动，ture：启动，false：停止
 */
using ScProcessStateCallback = std::function<void(int type, bool isStart)>;

/**
 * @brief The CDAnalysisReportCallback type 分析结果回调函数
 * @param type			[in]处理类型
 * @param info			[in]报告的信息
 * @param size			[in]信息长度
 */
using CDAnalysisReportCallback = std::function<void(int type, const char* const info, int size)>;

/**
 * @brief The CDMessageCallback type 报文回调函数
 * @param status		[in]报文类型
 * @param info			[in]报文信息
 */
using CDMessageCallback = std::function<void(Sc::Status status, const ScString& message)>;

/**
 * @brief The ScFeedbackMetaDataCallback type 信号元数据反馈回调函数
 * @param service		[in]反馈服务
 * @param id			[in]用户ID
 * @param channel		[in]通道号
 * @param meta			[in]信号元数据
 */
using ScFeedbackMetaDataCallback = std::function<void(class ScAbstractService* service, const usrid &id, int channel, const ScMetaData &meta)>;

/**
 * @brief The ScNextFileNameCallback type 获取下一个文件名的回调函数类型
 * @param fileName		[in]文件名
 * @param order			[in]文件顺序
 * @param dataFormat	[in]数据格式
 * @param strategy		[in]数据头重新验证的策略
 */
using ScNextFileNameCallback = std::function<ScString(const ScString &, scuint64, Sc::DataFormat, Sc::HeaderStrategy &)>;

/**
 * The ScCallbackManager struct 回调函数管理器
 */
struct ScCallbackManager
{
	ScNextFileNameCallback nextFileNameCallback{ };			/**< 获取下一个文件名的回调函数 */
	ScFeedbackMetaDataCallback feedbackMetaDataCallback{ };	/**< 信号元数据反馈回调函数 */

	static ScCallbackManager *instance() {
		static ScCallbackManager mgr;
		return &mgr;
	}

private:
	ScCallbackManager() = default;
	~ScCallbackManager() = default;
	SC_DISABLE_COPY_MOVE(ScCallbackManager)
};

#define Sc_CALLBACK ScCallbackManager::instance()

SC_BEGIN_NAMESPACE

/**
 * @brief sendCarrierData 发送载波数据
 * @param graphicCallback		[in]接受图形数据的回调函数
 * @param dataType				[in]数据类型
 * @param data					[in]图形数据
 * @param length				[in]数据长度
 */
template<typename T>
void sendCarrierData(ScGraphicCallback graphicCallback, Sc::DataType dataType, const T* data, int length) {
	if (graphicCallback && data && length > 0)
		graphicCallback(dataType, ScTypeToBitDepth<T>::value, reinterpret_cast<const char*>(data), length * sizeof(T));
}

SC_END_NAMESPACE

#endif // SCCALLBACKMANAGER_H
