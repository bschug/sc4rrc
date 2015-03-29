#ifndef SC4RRC_CONFIG_HPP
#define SC4RRC_CONFIG_HPP

#ifdef _WIN32
 #ifndef SC4RRC_LIB
  #define SC4RRC_API __declspec(dllimport)
 #else
  #define SC4RRC_API __declspec(dllexport)
 #endif
#else
 #define SC4RRC_API
#endif

#pragma warning(disable:4251)

#endif