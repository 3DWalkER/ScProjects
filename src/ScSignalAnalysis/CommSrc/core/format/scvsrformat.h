#ifndef SCVSRFORMAT_H
#define SCVSRFORMAT_H

#include "scsignalformat.h"

#pragma pack(1)

struct VSR_HEADER
{
    char           record_label[4];							/**< 记录标签 "RDEF"  0b01000110010001010100010001010010 */	  
    unsigned int   record_length;							/**< 记录长度 = 2 * (sample_rate * sample_size) / 8 + header_size；header_size = 176  */
    unsigned short record_version_id;						/**< 记录版本标识(当前版本为1)版本号应与观测文件中给出的保持一致  */
    unsigned short station_id;								/**< 站点标识 */
    unsigned short spacecraft_id;							/**< 航天器标识 */
    unsigned short sample_size;								/**< 样本大小，样本bit数：1/2/4/8/16 */
    unsigned int   sample_rate;								/**< 采样率，采样率*2*样本大小应为32的倍数，以保持样本字长为32位   指定本记录中数据的采样率，单位为每秒复数样本数 */
    unsigned short validity_flag;							/**< 有效性标志，值为0表示无错误 */
    unsigned short agency_flag;								/**< 机构标志，值为0表示此字段未启用 1=ESA 2=JAXA 3=NASA */
    double		   rf_to_if_downconv;						/**< 分辨率Hz，从射频到中频 */
    double		   if_to_channel_downconv;					/**< 中心频率 1 micro-Hz，从中频到信道中心频率 */
    unsigned short time_tag_year;							/**< 时间标签年份 */
    unsigned short time_tag_boy;							/**< 时间标签年积日：1-366 */
    unsigned int   time_tag_second_of_day;					/**< 日秒时间标签：0-86400 */
    double	       time_tag_picoseconds_of_the_second;		/**< 秒内皮秒时间标签，当首个数据样本时间与秒起始点存在已知延迟时，使用正非零值。若无延迟则设为0。 */
    double		   channel_accumulated_phase;				/**< 信道累积相位 */
    double		   channel_phase_polynpmial_coefficient0;	/**< 信道相位多项式系0 */
    double		   channel_phase_ploynomial_coefficient1;	/**< 信道相位多项式系1 */
    double		   channel_phase_ploynomial_coefficient2;	/**< 信道相位多项式系2 */
    double		   channel_phase_ploynomial_coefficient3;	/**< 信道相位多项式系3 */
    char		   empty_fields_future_extension[36];		/**< 特殊数据格式扩展 */
    char		   empty_fields_internal_agency_use[40];	/**< 机构内部使用 */
    int		       end_label;								/**< 结束标签：-99999 */
};

#pragma pack()

class ScVsrFormat : public ScSignalFormat
{
public:
    ScVsrFormat();
    ~ScVsrFormat() = default;

    bool isValid(const char* inBuff, int inlen) override;
	int headerLength() const override { return sizeof(VSR_HEADER); }
    double ratio() override { return bitDepth() <= 0 ? 1.0 : (8.0 / bitDepth()); }
};

#endif // SCVSRFORMAT_H

