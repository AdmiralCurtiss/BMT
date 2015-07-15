#ifndef __BMT__UTILITY_H__
#define __BMT__UTILITY_H__

#include <string>

std::wstring  BMT_GetDocumentsDir   (void);
bool          BMT_GetUserProfileDir (wchar_t* buf, uint32_t* pdwLen);
bool          BMT_IsTrue            (const wchar_t* string);
bool          BMT_IsAdmin           (void);
int           BMT_MessageBox        (std::wstring caption, std::wstring title, uint32_t flags);

inline size_t
BMT_NextPowerOfTwo (size_t x)
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

#endif /* __BMT__UTILITY_H__ */