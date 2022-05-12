// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.200729.8

#ifndef WINRT_Windows_Graphics_Printing_2_H
#define WINRT_Windows_Graphics_Printing_2_H
#include "winrt/impl/Windows.Foundation.2.h"
#include "winrt/impl/Windows.Graphics.Printing.1.h"
WINRT_EXPORT namespace winrt::Windows::Graphics::Printing
{
    struct PrintTaskSourceRequestedHandler : Windows::Foundation::IUnknown
    {
        PrintTaskSourceRequestedHandler(std::nullptr_t = nullptr) noexcept {}
        PrintTaskSourceRequestedHandler(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Foundation::IUnknown(ptr, take_ownership_from_abi) {}
        template <typename L> PrintTaskSourceRequestedHandler(L lambda);
        template <typename F> PrintTaskSourceRequestedHandler(F* function);
        template <typename O, typename M> PrintTaskSourceRequestedHandler(O* object, M method);
        template <typename O, typename M> PrintTaskSourceRequestedHandler(com_ptr<O>&& object, M method);
        template <typename O, typename M> PrintTaskSourceRequestedHandler(weak_ref<O>&& object, M method);
        auto operator()(Windows::Graphics::Printing::PrintTaskSourceRequestedArgs const& args) const;
    };
    struct PrintPageDescription
    {
        Windows::Foundation::Size PageSize;
        Windows::Foundation::Rect ImageableRect;
        uint32_t DpiX;
        uint32_t DpiY;
    };
    inline bool operator==(PrintPageDescription const& left, PrintPageDescription const& right) noexcept
    {
        return left.PageSize == right.PageSize && left.ImageableRect == right.ImageableRect && left.DpiX == right.DpiX && left.DpiY == right.DpiY;
    }
    inline bool operator!=(PrintPageDescription const& left, PrintPageDescription const& right) noexcept
    {
        return !(left == right);
    }
    struct __declspec(empty_bases) PrintManager : Windows::Graphics::Printing::IPrintManager
    {
        PrintManager(std::nullptr_t) noexcept {}
        PrintManager(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintManager(ptr, take_ownership_from_abi) {}
        static auto GetForCurrentView();
        static auto ShowPrintUIAsync();
        static auto IsSupported();
    };
    struct __declspec(empty_bases) PrintPageInfo : Windows::Graphics::Printing::IPrintPageInfo
    {
        PrintPageInfo(std::nullptr_t) noexcept {}
        PrintPageInfo(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintPageInfo(ptr, take_ownership_from_abi) {}
        PrintPageInfo();
    };
    struct __declspec(empty_bases) PrintPageRange : Windows::Graphics::Printing::IPrintPageRange
    {
        PrintPageRange(std::nullptr_t) noexcept {}
        PrintPageRange(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintPageRange(ptr, take_ownership_from_abi) {}
        PrintPageRange(int32_t firstPage, int32_t lastPage);
        explicit PrintPageRange(int32_t page);
    };
    struct __declspec(empty_bases) PrintPageRangeOptions : Windows::Graphics::Printing::IPrintPageRangeOptions
    {
        PrintPageRangeOptions(std::nullptr_t) noexcept {}
        PrintPageRangeOptions(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintPageRangeOptions(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTask : Windows::Graphics::Printing::IPrintTask,
        impl::require<PrintTask, Windows::Graphics::Printing::IPrintTaskTargetDeviceSupport, Windows::Graphics::Printing::IPrintTask2>
    {
        PrintTask(std::nullptr_t) noexcept {}
        PrintTask(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTask(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskCompletedEventArgs : Windows::Graphics::Printing::IPrintTaskCompletedEventArgs
    {
        PrintTaskCompletedEventArgs(std::nullptr_t) noexcept {}
        PrintTaskCompletedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskCompletedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskOptions : Windows::Graphics::Printing::IPrintTaskOptionsCore,
        impl::require<PrintTaskOptions, Windows::Graphics::Printing::IPrintTaskOptionsCoreProperties, Windows::Graphics::Printing::IPrintTaskOptionsCoreUIConfiguration, Windows::Graphics::Printing::IPrintTaskOptions, Windows::Graphics::Printing::IPrintTaskOptions2>
    {
        PrintTaskOptions(std::nullptr_t) noexcept {}
        PrintTaskOptions(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskOptionsCore(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskProgressingEventArgs : Windows::Graphics::Printing::IPrintTaskProgressingEventArgs
    {
        PrintTaskProgressingEventArgs(std::nullptr_t) noexcept {}
        PrintTaskProgressingEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskProgressingEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskRequest : Windows::Graphics::Printing::IPrintTaskRequest
    {
        PrintTaskRequest(std::nullptr_t) noexcept {}
        PrintTaskRequest(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskRequest(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskRequestedDeferral : Windows::Graphics::Printing::IPrintTaskRequestedDeferral
    {
        PrintTaskRequestedDeferral(std::nullptr_t) noexcept {}
        PrintTaskRequestedDeferral(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskRequestedDeferral(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskRequestedEventArgs : Windows::Graphics::Printing::IPrintTaskRequestedEventArgs
    {
        PrintTaskRequestedEventArgs(std::nullptr_t) noexcept {}
        PrintTaskRequestedEventArgs(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskRequestedEventArgs(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskSourceRequestedArgs : Windows::Graphics::Printing::IPrintTaskSourceRequestedArgs
    {
        PrintTaskSourceRequestedArgs(std::nullptr_t) noexcept {}
        PrintTaskSourceRequestedArgs(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskSourceRequestedArgs(ptr, take_ownership_from_abi) {}
    };
    struct __declspec(empty_bases) PrintTaskSourceRequestedDeferral : Windows::Graphics::Printing::IPrintTaskSourceRequestedDeferral
    {
        PrintTaskSourceRequestedDeferral(std::nullptr_t) noexcept {}
        PrintTaskSourceRequestedDeferral(void* ptr, take_ownership_from_abi_t) noexcept : Windows::Graphics::Printing::IPrintTaskSourceRequestedDeferral(ptr, take_ownership_from_abi) {}
    };
    struct StandardPrintTaskOptions
    {
        StandardPrintTaskOptions() = delete;
        [[nodiscard]] static auto MediaSize();
        [[nodiscard]] static auto MediaType();
        [[nodiscard]] static auto Orientation();
        [[nodiscard]] static auto PrintQuality();
        [[nodiscard]] static auto ColorMode();
        [[nodiscard]] static auto Duplex();
        [[nodiscard]] static auto Collation();
        [[nodiscard]] static auto Staple();
        [[nodiscard]] static auto HolePunch();
        [[nodiscard]] static auto Binding();
        [[nodiscard]] static auto Copies();
        [[nodiscard]] static auto NUp();
        [[nodiscard]] static auto InputBin();
        [[nodiscard]] static auto Bordering();
        [[nodiscard]] static auto CustomPageRanges();
    };
}
#endif
