// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.200729.8

#ifndef WINRT_Windows_Management_Core_1_H
#define WINRT_Windows_Management_Core_1_H
#include "winrt/impl/Windows.Management.Core.0.h"
WINRT_EXPORT namespace winrt::Windows::Management::Core
{
    struct __declspec(empty_bases) IApplicationDataManager :
        Windows::Foundation::IInspectable,
        impl::consume_t<IApplicationDataManager>
    {
        IApplicationDataManager(std::nullptr_t = nullptr) noexcept {}
        IApplicationDataManager(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) IApplicationDataManagerStatics :
        Windows::Foundation::IInspectable,
        impl::consume_t<IApplicationDataManagerStatics>
    {
        IApplicationDataManagerStatics(std::nullptr_t = nullptr) noexcept {}
        IApplicationDataManagerStatics(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Foundation::IInspectable(ptr, take_ownership_from_abi) {}
    };
}
#endif
