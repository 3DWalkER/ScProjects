#ifndef SCCONTAINERUTILS_H
#define SCCONTAINERUTILS_H

#include "scutils/scglobal.h"
#include <iterator>

SC_BEGIN_NAMESPACE

template <typename Iterator>
using IfIsInputIterator = typename std::enable_if<
	std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value,
	bool>::type;

SC_END_NAMESPACE

#endif // SCCONTAINERUTILS_H