#pragma once

#include "em/meta/detect_bases.h"
#include "em/refl/common.h"

namespace em::Refl::Bases
{
    namespace detail
    {
        template <typename T>
        struct DefaultTraits
        {
            template <typename U = T> static constexpr auto AllBasesFlat()          {return Meta::DetectBases::AllBasesFlat<Refl::detail::StructBasesTag, U>{};}
            template <typename U = T> static constexpr auto VirtualBasesFlat()      {return Meta::DetectBases::VirtualBasesFlat<Refl::detail::StructBasesTag, U>{};}
            template <typename U = T> static constexpr auto NonVirtualBasesFlat()   {return Meta::DetectBases::NonVirtualBasesFlat<Refl::detail::StructBasesTag, U>{};}
            template <typename U = T> static constexpr auto NonVirtualBasesDirect() {return Meta::DetectBases::NonVirtualBasesDirect<Refl::detail::StructBasesTag, U>{};}

            template <typename U = T> static constexpr bool HaveBases()
            {
                // Not 100% sure what list is more optimal to check here. We could check either `AllBasesFlat` or both `VirtualBasesFlat` and `NonVirtualBasesFlat`.
                return !std::is_same_v<decltype(AllBasesFlat<U>()), Meta::TypeList<>>;
            }
        };

        template <typename T>
        struct SelectTraits {using type = DefaultTraits<std::remove_cvref_t<T>>;};
        template <typename T> requires requires{_adl_em_refl_Bases(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<T> *>());}
        struct SelectTraits<T> {using type = decltype(_adl_em_refl_Bases(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<T> *>()));};

        template <typename T>
        using Traits = typename SelectTraits<T>::type;
    }

    template <typename T> concept HasBases = detail::Traits<T>::HaveBases();

    // Various base lists. Never contain duplicates.
    // The `...AndSelf` versions include the same type as the last element, others don't.
    // The `...Flat` versions return all bases recursively, while `...Direct` only acts on direct bases.
    // The `All...` versions combine virtual and non-virtual bases, while `Virtual...` and `NonVirtual...` handle them separately.
    template <typename T> using AllBasesFlat          = decltype(detail::Traits<T>::AllBasesFlat());
    template <typename T> using VirtualBasesFlat      = decltype(detail::Traits<T>::VirtualBasesFlat());
    template <typename T> using NonVirtualBasesFlat   = decltype(detail::Traits<T>::NonVirtualBasesFlat());
    template <typename T> using NonVirtualBasesDirect = decltype(detail::Traits<T>::NonVirtualBasesDirect());
    template <typename T> using AllBasesFlatAndSelf          = Meta::list_append_types<AllBasesFlat<T>, T>;
    template <typename T> using VirtualBasesFlatAndSelf      = Meta::list_append_types<VirtualBasesFlat<T>, T>;
    template <typename T> using NonVirtualBasesFlatAndSelf   = Meta::list_append_types<NonVirtualBasesFlat<T>, T>;
    template <typename T> using NonVirtualBasesDirectAndSelf = Meta::list_append_types<NonVirtualBasesDirect<T>, T>;
}
