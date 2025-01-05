#pragma once

// Macros to declare reflected structs and struct members.
// The templates to read the reflection metadata are elsewhere, in `refl/api/struct.h`.

#include "em/macros/meta/comma.h"
#include "em/macros/meta/common.h"
#include "em/macros/meta/detectable_base.h"
#include "em/macros/meta/enclosing_class.h"
#include "em/macros/meta/if_else.h"
#include "em/macros/meta/ranges.h"
#include "em/macros/meta/sequence_for.h"
#include "em/macros/utils/forward.h"
#include "em/meta/lists.h"
#include "em/meta/packs.h"
#include "em/meta/qualifiers.h"
#include "em/meta/stateful/list.h"
#include "em/meta/tags.h"
#include "em/refl/common.h"

#include <concepts>


namespace em::Refl
{
    namespace detail
    {
        // Given a `type, attributes...` list (as accepted by `EM_REFL`), returns the `type` from it.
        template <typename Type, Attribute ...Attrs>
        using MemberType = Type;
    }

    // Describes a struct member type and a list of attributes associated with it.
    // When customizing this, pass `Type = void` to avoid overriding the type (to use `decltype(...)` on the getter as by default).
    template <typename Type, Attribute ...Attrs>
    struct MemberInfo
    {
        using type = Type;
        using attrs = Meta::TypeList<Attrs...>;
    };
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
 *             (type)(name, init)         // With initializer.
 *             (type, attr1, attr2)(name) // With attributes. Those are types derived from `em::Refl::BasicAttribute`. The initializer is allowed here too of course.
 *         )
 *     }
 *
 * The default initializer is `{}`. Use `(name,)` to remove any initializer (e.g. if the type is not default-constructible).
 */
#define EM_REFL(...) DETAIL_EM_REFL(EM_SEQ_GROUP2(__VA_ARGS__))
#define DETAIL_EM_REFL(seq_) DETAIL_EM_REFL_EMIT_MEMBERS(seq_) DETAIL_EM_REFL_EMIT_METADATA(seq_)

// `EM_STRUCT(name_etc)(...)` is a shorthand for `struct name_etc {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_STRUCT(.../*name_etc*/) struct __VA_ARGS__ DETAIL_EM_STRUCT_BODY
// `EM_CLASS(name_etc)(...)` is a shorthand for `struct class {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_CLASS(.../*name_etc*/) class __VA_ARGS__ DETAIL_EM_STRUCT_BODY
#define DETAIL_EM_STRUCT_BODY(...) { EM_REFL(__VA_ARGS__) };

// For custom annotations in `EM_REFL(...)`.
// `...` gets pasted into the class verbatim.
// `category_` is a single-word ID that roughly explains what this is. `data_` can be anything.
// `...`, if specified, is inserted literally into the class, inbetween the members.
#define EM_VERBATIM_LOW(category_, data_, .../*verbatim*/) (_em_verbatim)(category_,data_,__VA_ARGS__)

// Takes a member list (preprocessed with `EM_SEQ_GROUP2(...)`) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq_) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL)(p_type_attrs_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL(p_type_attrs_, .../*name_ [,init_...]*/) ::em::Refl::detail::MemberType<EM_IDENTITY p_type_attrs_> EM_VA_FIRST(__VA_ARGS__) EM_IF_COMMA(__VA_ARGS__)(DETAIL_EM_REFL_INITIALIZER(__VA_ARGS__))({});
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM(p_type_attrs_, category_, data_, .../*text_*/) __VA_ARGS__
#define DETAIL_EM_REFL_INITIALIZER(unused_, ...) __VA_OPT__(= __VA_ARGS__)

// If `p_type_attrs_` is `_em_verbatim`, returns `a`. Otherwise returns `b`.
// `DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(a)(b)`
#define DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REFL_IF_VERBATIM_, EM_IDENTITY p_type_attrs_)
#define DETAIL_EM_REFL_IF_VERBATIM__em_verbatim ,

// Generates metadata for a class. `seq_` is the list of members, preprocessed with `EM_SEQ_GROUP2(...)`.
#define DETAIL_EM_REFL_EMIT_METADATA(seq_) \
    /* Typedef the enclosing class. */\
    EM_TYPEDEF_ENCLOSING_CLASS(_em_refl_Self) \
    \
    /* Detect this as a base class. */\
    EM_DETECTABLE_BASE((::em::Refl::detail::StructBasesTag), (_em_refl_Self)) \
    \
    /* In all those functions we're currently using `same_ignoring_cvref<Self>` to reject derived types. */\
    /* This sounds like a sensible behavior, but if we ever decide to change it, simply accepting a reference to `_em_refl_Self` should work. */\
    /* Except for `_adl_em_refl_GetMember`, which needs perfect forwarding (would probably have to pass the qualifiers separately). */\
    \
    /* Member count. */\
    [[nodiscard]] friend constexpr int _adl_em_refl_NumMembers(int/*AdlDummy*/, const ::em::Meta::same_ignoring_cvref<_em_refl_Self> auto *) \
    { \
        return EM_END(DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A seq_); \
    } \
    /* A getter for the members. */\
    template <int _em_I> \
    [[nodiscard]] friend constexpr auto &&_adl_em_refl_GetMember(int/*AdlDummy*/, ::em::Meta::same_ignoring_cvref<_em_refl_Self> auto &&_em_self) \
    { \
        DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq_) \
        static_assert(::em::Meta::always_false<decltype(_em_self), ::em::Meta::ValueTag<_em_I>>, "Member index is out of range."); \
    } \
    /* Return a `TypeList` of lists of member attributes. */\
    template <int _em_I> \
    [[nodiscard]] friend constexpr auto _adl_em_refl_GetMemberInfo(int/*AdlDummy*/, const ::em::Meta::same_ignoring_cvref<_em_refl_Self> auto *) \
    { \
        return ::em::Meta::list_type_at<::em::Meta::TypeList<EM_REMOVE_LEADING_COMMA(EM_END(DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A seq_))>, _em_I>{}; \
    }
// A loop to count the members.
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)()(+1)
// A loop to emit `if constexpr (i == counter) return member; else` for all members, to return references to them.
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP, SF_NULL, 0, seq)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP(n, counter_, p_type_attrs_, name_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)(counter_)(counter_+1, if constexpr (_em_I == counter_) return EM_FWD(_em_self).name_; else)
// A loop to emit a list of lists of member attributes. (The outer list has one element per member.)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(p_type_attrs_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_attrs_)()(, ::em::Refl::MemberInfo<EM_IDENTITY p_type_attrs_>)

/* // This isn't need for anything right now, but can help adding extra commands to `EM_REFL(...)` later.
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
*/
