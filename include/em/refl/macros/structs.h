#pragma once

// Macros to declare reflected structs and struct members.
// The templates to read the reflection metadata are elsewhere, in `refl/api/structs.h`.

#include "em/macros/meta/comma.h"
#include "em/macros/meta/common.h"
#include "em/macros/meta/detectable_base.h"
#include "em/macros/meta/enclosing_class.h"
#include "em/macros/meta/if_else.h"
#include "em/macros/meta/ranges.h"
#include "em/macros/meta/sequence_for.h"
#include "em/macros/utils/forward.h"
#include "em/meta/cvref.h"
#include "em/meta/lists.h"
#include "em/meta/packs.h"
#include "em/meta/stateful/list.h"
#include "em/meta/tags.h"
#include "em/refl/common.h"

#include <array>
#include <concepts>
#include <string_view>


namespace em::Refl::Structs
{
    namespace detail::Macros
    {
        // Given a `type, attributes...` list (as accepted by `EM_REFL`), returns the `type` from it.
        template <typename Type, Attribute ...Attrs>
        using MemberType = Type;
    }
}

// Access modifiers for `EM_REFL(...)`.
#define EM_PUBLIC    EM_VERBATIM_LOW(access, public   , public:   )
#define EM_PRIVATE   EM_VERBATIM_LOW(access, private  , private:  )
#define EM_PROTECTED EM_VERBATIM_LOW(access, protected, protected:)
// Pastes a piece of code verbatim in `EM_REFL(...)`.
#define EM_VERBATIM(...) EM_VERBATIM_LOW(user,, __VA_ARGS__)

/* Declares the reflected members of a struct.
 * Example usage:
 *     struct A
 *     {
 *         EM_REFL(
 *             (type)(name)               // Basic declaration, the initializer defaults to `{}`.
 *         EM_PUBLIC                      // Access modifiers.
 *             (type)(name, 42)           // With initializer.
 *             (type, attr1, attr2)(name) // With attributes. Those are types derived from `em::Refl::BasicAttribute`. The initializer is allowed here too of course.
 *         )
 *     }
 *
 * The default initializer is `{}`. Use `(name,)` to remove any initializer (e.g. if the type is not default-constructible).
 */
#define EM_REFL(...) DETAIL_EM_REFL(DETAIL_EM_REFL_SPLIT_LIST(__VA_ARGS__))

// `EM_STRUCT(name_etc)(...)` is a shorthand for `struct name_etc {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_STRUCT(.../*name_etc*/) struct __VA_ARGS__ DETAIL_EM_STRUCT_BODY
// `EM_CLASS(name_etc)(...)` is a shorthand for `struct class {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_CLASS(.../*name_etc*/) class __VA_ARGS__ DETAIL_EM_STRUCT_BODY
#define DETAIL_EM_STRUCT_BODY(...) { EM_REFL(__VA_ARGS__) };

#define EM_UNNAMED_MEMBERS (,_em_unnamed_members)

// For custom annotations in `EM_REFL(...)`.
// `...` gets pasted into the class verbatim.
// `category_` is a single-word ID that roughly explains what this is. `data_` can be anything.
// `...`, if specified, is inserted literally into the class, inbetween the members.
#define EM_VERBATIM_LOW(category_, data_, .../*verbatim*/) (_em_verbatim)(category_,data_,__VA_ARGS__)


// --- Internals:

// The implementaiton of `EM_REFL(...)`.
#define DETAIL_EM_REFL(...) DETAIL_EM_REFL_2(__VA_ARGS__)
#define DETAIL_EM_REFL_2(control_, seq_) DETAIL_EM_REFL_3(EM_IDENTITY DETAIL_EM_REFL_CONTROL(control_), EM_SEQ_GROUP2(seq_))
#define DETAIL_EM_REFL_3(...) DETAIL_EM_REFL_4(__VA_ARGS__)
#define DETAIL_EM_REFL_4(named_members_, /*add new control vars here*/ seq_) DETAIL_EM_REFL_EMIT_MEMBERS(seq_) DETAIL_EM_REFL_EMIT_METADATA(named_members_, seq_)

#define DETAIL_EM_REFL_CONTROL(control_) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_CONTROL_STEP, SF_STATE, (1/*enable member names*/), control_)
#define DETAIL_EM_REFL_CONTROL_STEP(n, d, unused_, command_, ...) EM_CALL(EM_CAT(DETAIL_EM_REFL_CONTROL_STEP_, command_), EM_IDENTITY d __VA_OPT__(,) __VA_ARGS__)
#define DETAIL_EM_REFL_CONTROL_STEP__em_unnamed_members(x, ...) (EM_IF_01(x)(0)(EM_FAIL("Duplicate `EM_UNNAMED_MEMEBRS`")) __VA_OPT__(,) __VA_ARGS__)

// Takes a member list (preprocessed with `EM_SEQ_GROUP2(...)`) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq_) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL)(p_type_attrs_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL(p_type_attrs_, .../*name_ [,init_...]*/) ::em::Refl::Structs::detail::Macros::MemberType<EM_IDENTITY p_type_attrs_> EM_VA_FIRST(__VA_ARGS__) EM_IF_COMMA(__VA_ARGS__)(DETAIL_EM_REFL_INITIALIZER(__VA_ARGS__))({});
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM(p_type_attrs_, category_, data_, .../*text_*/) __VA_ARGS__
#define DETAIL_EM_REFL_INITIALIZER(unused_, ...) __VA_OPT__(= __VA_ARGS__)

// If `p_type_attrs_` is `_em_verbatim`, returns `a`. Otherwise returns `b`.
// `DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(a)(b)`
#define DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REFL_IF_VERBATIM_, EM_IDENTITY p_type_attrs_)
#define DETAIL_EM_REFL_IF_VERBATIM__em_verbatim ,

// Generates metadata for a class. `seq_` is the list of members, preprocessed with `EM_SEQ_GROUP2(...)`.
// `named_members_` is 0 or 1, whether to generate the member name information.
#define DETAIL_EM_REFL_EMIT_METADATA(named_members_, seq_) \
    /* Typedef the enclosing class. */\
    EM_TYPEDEF_ENCLOSING_CLASS(_em_refl_Self) \
    \
    /* Detect this as a base class. */\
    EM_DETECTABLE_BASE((::em::Refl::detail::StructBasesTag), (_em_refl_Self)) \
    \
    struct _em_refl_Traits \
    { \
        /* Member count. This is a fan */\
        static constexpr int num_members = 0 EM_END(DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A seq_); \
        /* A getter for the members. */\
        /* Here we return a reference, but custom classes can also return by value here. */\
        template <int _em_I> \
        static constexpr auto &&GetMember(auto &&_em_self) \
        { \
            DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq_) \
            static_assert(::em::Meta::always_false<decltype(_em_self), ::em::Meta::ValueTag<_em_I>>, "Member index is out of range."); \
        } \
        /* [optional] Return something with `::type` to indicate a member type (omit or `void` to guess),
        // and with `::attrs` with a type list of attributes (the list can be any variadic template, omit if no attributes). */\
        template <int _em_I> \
        static constexpr auto GetMemberInfo() \
        { \
            return ::em::Meta::list_type_at<::em::Meta::TypeList<EM_REMOVE_LEADING_COMMA(EM_END(DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A seq_))>, _em_I>{}; \
        } \
        /* [optional] Return the member name. Omit the function to indicate the lack of names. */\
        /* Currently the convention is to return null-terminated strings. The library itself doesn't use that for anything, but that's heavily recommended. */\
        EM_IF_01(named_members_)( \
            static constexpr ::std::string_view GetMemberName(int _em_i) \
            { \
                static constexpr ::std::array<::std::string_view, num_members> _em_array = { EM_END(DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A seq_) }; \
                return _em_array[_em_i]; \
            } \
        )() \
    }; \
    /* We're currently using `same_ignoring_cvref<Self>` to reject derived types. */\
    /* Replacing this with `_em_refl_Self` would allow derived types, but that seems to be worse. */\
    friend constexpr _em_refl_Traits _adl_em_refl_Struct(int/*AdlDummy*/, const ::em::Meta::same_ignoring_cvref<_em_refl_Self> auto *) {return {};}

// A loop to count the members.
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)()(+1)
// A loop to emit `if constexpr (i == counter) return member; else` for all members, to return references to them.
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP, SF_NULL, 0, seq)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP(n, counter_, p_type_attrs_, name_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(counter_)(counter_+1, if constexpr (_em_I == counter_) return EM_FWD(_em_self).name_; else)
// A loop to emit a list of lists of member types and attributes.
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)()(, ::em::Refl::MemberInfo<EM_IDENTITY p_type_attrs_>)
// A loop to emit a list of lists of member names.
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(p_type_attrs_, name_, ...) #name_,

// Given a list `(,a)(,b)(,c)(d)(e)(f)`, inserts a comma to produce `(,a)(,b)(,c) , (d)(e)(f)`.
// Either half can be empty, in that case the comma can be the first and/or the last thing in the return value.
#define DETAIL_EM_REFL_SPLIT_LIST(seq_) EM_END(DETAIL_EM_REFL_SPLIT_LIST_LOOP_A seq_)
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A(...) DETAIL_EM_REFL_SPLIT_LIST_LOOP_A_((__VA_ARGS__), EM_VA_FIRST(__VA_ARGS__))
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B(...) DETAIL_EM_REFL_SPLIT_LIST_LOOP_B_((__VA_ARGS__), EM_VA_FIRST(__VA_ARGS__))
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A0(...) (__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_B0
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B0(...) (__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_A0
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A_(x, ...) __VA_OPT__(,) x EM_CAT(DETAIL_EM_REFL_SPLIT_LIST_LOOP_B,__VA_OPT__(0))
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B_(x, ...) __VA_OPT__(,) x EM_CAT(DETAIL_EM_REFL_SPLIT_LIST_LOOP_A,__VA_OPT__(0))
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A_END ,
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B_END ,
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A0_END
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B0_END
