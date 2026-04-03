#include "../3rdparty/mainui_cpp/miniutl/utlstring.h"

void CUtlString::AssertStringTooLong()
{
	AssertMsg(false, "Assertion failed: length > k_cchMaxString");
}
