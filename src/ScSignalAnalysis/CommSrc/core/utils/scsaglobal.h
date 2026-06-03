#ifndef SCSAGLOBAL_H
#define SCSAGLOBAL_H

#include "scutils/text/scstring.h"

#define		LEN256M     268435456
#define		LEN128M     134217728
#define		LEN64M      67108864
#define		LEN32M      33554432
#define		LEN16M      16777216
#define		LEN15M      15728640
#define		LEN10M      10485760
#define		LEN8M       8388608
#define		LEN4M       4194304
#define		LEN3M		3145728
#define		LEN2M       2097152
#define		LEN1M       1048576
#define		LEN512K     524288
#define		LEN256K     262144
#define		LEN128K     131072
#define		LEN64K      65536
#define		LEN32K      32768
#define		LEN16K      16384
#define		LEN8K       8192
#define		LEN4K       4096
#define		LEN2K       2048
#define		LEN1K       1024

/**
 * @brief The SAHANDLE macro 处理句柄
 */
using SAHANDLE = void*;

/**
 * @brief The usrid macro 自定义用户ID
 */
using usrid = ScString;

/**
 * @brief The Sc_DEBUG macro 调试信息输出
 */
#ifdef SC_ENABLE_DEBUG_OUTPUT
#   define SA_DEBUG(Msg, ...) SC_DEBUG("【{} {}】" Msg, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#   define SA_DEBUG(Msg, ...) ((void)0)
#endif

SC_BEGIN_NAMESPACE

/**
 * @brief The SC_ERROR macro 错误代码
 */
enum ErrorCode
{
	SUCCESS,					/**< 无错误 */
	HANDLE_IS_NULL,				/**< 句柄为空 */
	MEMORY_ERROR,				/**< 内存申请失败 */
	MEMORY_OVERFLOW,			/**< 内存溢出 */
	SIGNAL_BASIC_PARAM_INVALID,	/**< 信号基础参数无效 */
	SYMBOL_RATE_IS_ZERO,		/**< 符号速率为0 */
	DATA_LENGTH_NOT_MEET,		/**< 数据长度不满足要求 */
	DATA_BUFFER_INVALID,		/**< 数据缓存无效 */
	TEMP_FILE_CREATE_FAILED,	/**< 临时文件创建失败 */
	ScMA_PRIM_FILE_NOT_FOUND,   /**< PRIM文件不存在 */
	DATA_MGR_OPEN_FAILED,		/**< 数据管理器打开失败 */
	UNDEFINE_ERROR				/**< 未定义错误 */
};

/**
 * @brief The Status enum 状态信息
 */
enum Status
{
	S_Start,        /**< 启动 */
	S_Starting,		/**< 启动中 */
	S_Running,      /**< 运行中 */
	S_Stoping,      /**< 停止中 */
	S_Stop,         /**< 停止 */
	S_Info,         /**< 信息 */
	S_Progress,     /**< 进度 */
	S_Error,        /**< 错误 */
	S_Debug,        /**< 调试 */
	S_Warn,         /**< 警告 */
	S_Critical,     /**< 致命错误 */
	S_Result        /**< 运行结果 */
};

/**
 * @brief The DataSource enum 数据来源
 */
enum DataSource
{
	DS_Sample,      /**< 样本 */
	DS_MinIO,       /**< MinIO */
	DS_Increment,	/**< 文件增量 */
	DS_Device,      /**< 设备 */
	DS_User			/**< 用户 */
};

/**
 * @brief The DataSource enum 数据来源
 */
enum DataType
{
	DT_Unknown,		/**< 未知 */
	DT_Real,		/**< 实信号 */
	DT_IQ,			/**< 复信号 */
	DT_Demod,		/**< 解调数据 */
	DT_Decode,		/**< 译码数据 */
	DT_FFT,			/**< FFT数据 */
	DT_Rate,		/**< 速率 */
	DT_Freq,		/**< 频率 */
	DT_Constell,	/**< 星座 */
	DT_Vector,		/**< 矢量 */
	DT_Eye,			/**< 眼图 */
	DT_Period,		/**< 周期 */
	DT_Doppler,		/**< 多普勒 */
	DT_Voice		/**< 话音 */
};

/**
 * @brief The DataFormat enum 数据格式
 */
enum DataFormat
{
	DF_Unknown,		/**< 未知格式 */
	DF_None,		/**< 无格式 */
	DF_Wave,		/**< 音频 */
	DF_VSI,			/**< VSI，实信号 */
	DF_VSR,			/**< VSR，复信号 */
	DF_Custom		/**< 自定义 */
};

/**
 * @brief The DataBlockMode enum 数据块类型
 */
enum DataBlockMode
{
	DBM_Unknown,	/**< 未知的 */
	DBM_Single,		/**< 单个数据块 */
	DBM_Multiple	/**< 多个数据块 */
};

/**
 * @brief 数据头重新验证策略
 */
enum HeaderStrategy
{
	HS_None,		/**< 保持当前状态，不做任何处理 */
	HS_Repeat,		/**< 重新验证 */
	HS_Search		/**< 搜索头 */
};

/**
 * @brief The BitDepth enum 量化别特数
 */
enum BitDepth
{
	BD_Unknown = 0,		/**< 未知 */
	BD_Bit1 = 1,		/**< 1 bit*/
	BD_Bit2 = 2,		/**< 2 bit */
	BD_Bit4 = 4,		/**< 4 bit */
	BD_Bit8 = 8,		/**< char */
	BD_Bit16 = 16,		/**< short */
	BD_Bit32 = 32,		/**< flaot */
	BD_Bit64 = 64		/**< double */
};

/**
 * @brief The ReadMode enum 数据读取模式
 */
enum ReadMode
{
	RM_Sequential,	/**< 连续读取 */
	RM_Discrete,    /**< 非连续读取 */
	RM_ReadOnce		/**< 读取一次 */
};

/**
 * @brief The ConvertMode enum 数据转换模式
 */
enum ConvertMode
{
	CM_Normal,		/**< 常规转换，如char->short等 */
	CM_PerByte		/**< 按照转换比例逐个遍历缓冲区 */
};

SC_END_NAMESPACE

/**
 * @brief The ScMetaData struct 信号元数据
 */
struct ScMetaData
{
	Sc::DataSource source{ Sc::DS_Sample };		/**< 数据来源 */
	Sc::DataFormat dataFormat{ Sc::DF_Unknown };/**< 数据格式 */
	Sc::DataType dataType{ Sc::DT_IQ };			/**< 数据类型 */
	Sc::BitDepth bitDepth{ Sc::BD_Bit16 };		/**< 量化比特数 */
	int channelCount{ 1 };						/**< 通道数 */
	double sampleRate{ 0.0 };					/**< 采样率 */
	ScString name{ };							/**< 文件/设备名称 */
};

/**
 * @brief The ScSaWorkParam struct 信号分析工作参数
 */
struct ScSaWorkParam
{
	ScMetaData meta;		/**< 信号源数据 */
};

/**
 * @brief The ScTypeToBitDepth struct 数值类型转量化比特数
 */
template <typename T>
struct ScTypeToBitDepth {
	static constexpr Sc::BitDepth value = Sc::BD_Unknown;
};
template<> struct ScTypeToBitDepth<char> { static constexpr Sc::BitDepth value = Sc::BD_Bit8; };
template<> struct ScTypeToBitDepth<short> { static constexpr Sc::BitDepth value = Sc::BD_Bit16; };
template<> struct ScTypeToBitDepth<int> { static constexpr Sc::BitDepth value = Sc::BD_Bit32; };
template<> struct ScTypeToBitDepth<float> { static constexpr Sc::BitDepth value = Sc::BD_Bit32; };
template<> struct ScTypeToBitDepth<double> { static constexpr Sc::BitDepth value = Sc::BD_Bit64; };

SC_BEGIN_NAMESPACE

/**
 * @brief copy  重复复制到指定长度
 * @param pDst          [ou]目标缓存区
 * @param dstlen		[in]目标长度
 * @param pSrc          [in]原始数据
 * @param srclen        [in]原始数据长度
 * @note 不考虑pDst和pSrc内存重叠的情况
 */
template <typename T>
void copy(T* pDst, int dstlen, const T* pSrc, int srclen)
{
	if (!pDst || dstlen <= 0 || !pSrc || srclen <= 0)
		return;

	int length = dstlen > srclen ? srclen : dstlen;
	if (pDst != pSrc)
		memcpy(pDst, pSrc, length * sizeof(T));

	int progress = length;
	while (progress < dstlen)
	{
		length = progress * 2 > dstlen ? dstlen - progress : progress;
		memcpy(pDst + progress, pDst, length * sizeof(T));
		progress += length;
	}
}

SC_END_NAMESPACE

#endif // SCSAGLOBAL_H
