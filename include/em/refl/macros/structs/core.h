#pragma once

// Macros to declare reflected structs and struct members.
// The templates to read the reflection metadata are elsewhere, in `refl/api/structs.h`.

#include "em/macros/meta/comma.h"
#include "em/macros/meta/common.h"
#include "em/macros/meta/detectable_base.h"
#include "em/macros/meta/enclosing_class.h"
#include "em/macros/meta/if_else.h"
#include "em/macros/meta/indexing.h"
#include "em/macros/meta/ranges.h"
#include "em/macros/meta/sequence_for.h"
#include "em/macros/utils/forward.h"
#include "em/meta/common.h" // IWYU pragma: keep, used in the macros.
#include "em/meta/lists.h" // IWYU pragma: keep, used in the macros.
#include "em/refl/common.h"

#include <array> // IWYU pragma: keep, used in the macros.
#include <cstddef> // IWYU pragma: keep, used in the macros.
#include <string_view> // IWYU pragma: keep, used in the macros.


namespace em::Refl::Structs
{
    namespace detail::Macros
    {
        // Given a `type, attributes...` list (as accepted by `EM_REFL`), returns the `type` from it.
        template <typename Type, Attribute ...Attrs>
        using MemberType = Type;
    }
}

/* Declares the reflected members of a struct.
 * Example usage:
 *     struct A
 *     {
 *         EM_REFL(
 *             (type)(name)               // Basic declaration, the initializer defaults to `{}`.
 *           EM_PUBLIC                    // Access modifiers.
 *             (type)(name, 42)           // With initializer.
 *             (type, attr1, attr2)(name) // With attributes. Those are types derived from `em::Refl::BasicAttribute`. The initializer is allowed here too of course.
 *         )
 *     }
 *
 * The default initializer is `{}`. Use `(name,)` to remove any initializer (e.g. if the type is not default-constructible).
 *
 * Before any members, you can specify zero or more control statements, such as `EM_UNNAMED_MEMBERS`,
 *   or any custom statements that you can implement yourself in terms of `EM_REFL_PREPROCESS_LOW()`.
 */
#define EM_REFL(...) DETAIL_EM_REFL(DETAIL_EM_REFL_SPLIT_LIST(__VA_ARGS__))

// `EM_STRUCT(name_etc)(...)` is a shorthand for `struct name_etc {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_STRUCT(.../*name_etc*/) struct __VA_ARGS__ DETAIL_EM_STRUCT_BODY
// `EM_CLASS(name_etc)(...)` is a shorthand for `struct class {EM_REFL(...)};`. `name_etc` can contain bases, attributes, etc.
#define EM_CLASS(.../*name_etc*/) class __VA_ARGS__ DETAIL_EM_STRUCT_BODY
#define DETAIL_EM_STRUCT_BODY(...) { EM_REFL(__VA_ARGS__) };

// Control statements that can appear at the beginning of `EM_REFL()`: [

// This optionally goes before the members in `EM_REFL`/`EM_STRUCT`/`EM_CLASS`.
// If specified, the member names are not emitted.
#define EM_UNNAMED_MEMBERS (,_em_unnamed_members)

// Will preprocess the entire member list of `EM_REFL()` (including verbatim blocks and annotations, but excluding control statements),
//   using those macros. Those are feed as parameters to `SF_FOR_EACH`.
// Each loop iteration receives `(...), name_or_kind [,...]`. If you want to leave this entity unchanged, you must emit it back, enclosed in `(...)`.
// You can also emit your own new entities, in the same format as explained below.
// Note that the format here is `((a...), b...)` despite `EM_REFL` operating on `(a...)(b...)`. They are grouped this way before being sent here.
// Call `EM_REFL_META_KIND(name_or_kind)` to get the entity kind: either a field, a verbatim block, or an annotation. Concat the resulting kind
//   to a macro name and dispatch based on that.
// If you got `field`:      then you receive `(type [,attrs...]), name [,init...]`. This declares a member variable.
// If you got `verbatim`:   then you receive `(category, data, text...), _em_verbatim`. This emits text verbatim in the same place as the member variables.
//                            For explanation of `category` and `data` see `EM_REFL_VERBATIM_LOW()`. `_em_verbatim` is a tag that appears literally.
// If you got `traits`:     then you receive `(text...), _em_traits`. This text is pasted verbatim into the traits class. `_em_traits` is a tag that
//                            appears literally. Note that this is the only entity kind that currently doesn't have a macro to directly emit it
//                            from `EM_REFL(...)`. To me it looks that it only makes sense to emit it from here.
// If you got `annotation`: then you receive `(category, error_if_unused, text...), _em_annotation`. This is a custom annotation that you can read here,
//                            in your macro. For explanation of `category` and `error_if_unused` see `EM_REFL_ANNOTATE_LOW()`. `_em_annotation` is a tag
//                            that appears literally.
// NOTE: When writing nested loops, you must respect the `n` parameter as usual!
// NOTE: Use `_em_refl_Self` to refer to the enclosing class.
// Debugging hint: you can temporarily comment out `DETAIL_EM_REFL_3` below to see the result of the preprocessing pass when expanding `EM_REFL()`.
#define EM_REFL_PREPROCESS_LOW(body_, step_, final_, data_) (,_em_preprocess, body_, step_, final_, data_)
// ]

// Things that can appear between the members of `EM_REFL()`: [

// Access modifiers for `EM_REFL(...)`.
#define EM_PUBLIC    EM_REFL_VERBATIM_LOW(access, public   , public:   )
#define EM_PRIVATE   EM_REFL_VERBATIM_LOW(access, private  , private:  )
#define EM_PROTECTED EM_REFL_VERBATIM_LOW(access, protected, protected:)
// Pastes a piece of code verbatim in `EM_REFL(...)`.
#define EM_VERBATIM(...) EM_REFL_VERBATIM_LOW(user,, __VA_ARGS__)

// Like `EM_VERBATIM()`, but also lets you customize the tags that verbatim blocks have.
// `...` gets pasted into the class verbatim.
// `category_` is a single-word ID that roughly explains what this is.
// `data_` can be anything, we don't access it directly.
#define EM_REFL_VERBATIM_LOW(category_, data_, .../*verbatim*/) (category_,data_,__VA_ARGS__)(_em_verbatim)
// Inserts a custom annotation in `EM_REFL()`.
// `category_` is a single-word ID that roughly explains what this is.
// `error_if_unused_` is either empty or a string literal. If not empty, `EM_REFL()` will emit this error message if no one removes it from
//   this annotation via `EM_REFL_PREPROCESS_LOW()`, which would indicate that the annotation is successfully consumed.
// `...` can be anything, we don't access it directly.
#define EM_REFL_ANNOTATE_LOW(category_, error_if_unused_, .../*data*/) (category_, error_if_unused_, __VA_ARGS__)(_em_annotation)

// ]


// --- Helpers for user modules, and for the internals:

// Given the element name (or the argument where it would be), possibly followed by a comma and unspecified stuff after that,
//   returns one of: `field`, `verbatim`, annotation`.
#define EM_REFL_META_KIND(...) DETAIL_EM_REFL_META_KIND_1(__VA_ARGS__)
#define DETAIL_EM_REFL_META_KIND_1(name_, ...) EM_VA_AT(1, EM_CAT(DETAIL_EM_REFL_META_KIND_2_, name_), field)
#define DETAIL_EM_REFL_META_KIND_2__em_verbatim   x, verbatim
#define DETAIL_EM_REFL_META_KIND_2__em_traits     x, traits
#define DETAIL_EM_REFL_META_KIND_2__em_annotation x, annotation


// --- Internals:

// The implementaiton of `EM_REFL(...)`.
#define DETAIL_EM_REFL(...) DETAIL_EM_REFL_2(__VA_ARGS__)
#define DETAIL_EM_REFL_2(control_, seq_) DETAIL_EM_REFL_3(SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_CONTROL_STEP, SF_STATE_EXPAND, (/*elems:*/EM_SEQ_GROUP2(seq_), /*enable member names:*/1), control_))
#define DETAIL_EM_REFL_CONTROL_STEP(n, d, empty_, command_, ...) DETAIL_EM_REFL_CONTROL_STEP_1(command_, EM_IDENTITY d __VA_OPT__(,) __VA_ARGS__)
#define DETAIL_EM_REFL_CONTROL_STEP_1(command_, ...) EM_CAT(DETAIL_EM_REFL_CONTROL_STEP_1_, command_)(__VA_ARGS__)
#define DETAIL_EM_REFL_CONTROL_STEP_1__em_unnamed_members(seq_, enable_member_names_) (seq_, EM_IF_01(enable_member_names_)(0)(EM_FAIL("Duplicate `EM_UNNAMED_MEMEBRS`")))
#define DETAIL_EM_REFL_CONTROL_STEP_1__em_preprocess(seq_, enable_member_names_, body_, step_, final_, data_) (SF_FOR_EACH0(body_, step_, final_, data_, seq_), enable_member_names_)
#define DETAIL_EM_REFL_3(...) DETAIL_EM_REFL_4(__VA_ARGS__)
// Here we emit metadata before the members, because at the end of members we can have some custom code injected by `EM_REFL_PREPROCESS_LOW()`,
//   which might expect the metadat to already exist.
// Having metadata before the members isn't an issue at all, because all the members are only named inside of functions, which can happen before they are declared.
#define DETAIL_EM_REFL_4(seq_, enable_member_names_) DETAIL_EM_REFL_EMIT_METADATA(seq_, enable_member_names_) DETAIL_EM_REFL_EMIT_MEMBERS(seq_)

#define DETAIL_EM_REFL_CONTROL(control_) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_CONTROL_STEP, SF_STATE, (1/*enable member names*/), control_)

// Takes a member list (preprocessed with `EM_SEQ_GROUP2(...)`) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq_) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(p_type_attrs_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_, EM_REFL_META_KIND(__VA_ARGS__))(p_type_attrs_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_field(p_type_attrs_, .../*name_ [,init_...]*/) ::em::Refl::Structs::detail::Macros::MemberType<EM_IDENTITY p_type_attrs_> EM_VA_FIRST(__VA_ARGS__) EM_IF_COMMA(__VA_ARGS__)(DETAIL_EM_REFL_INITIALIZER(__VA_ARGS__))({});
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_verbatim(p_content_, ...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_verbatim_1(EM_IDENTITY p_content_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_verbatim_1(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_verbatim_2(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_verbatim_2(category_, data_, .../*text*/) __VA_ARGS__
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_traits(...)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_annotation(p_content_, ...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_annotation_1(EM_IDENTITY p_content_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_annotation_1(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_annotation_2(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_annotation_2(category_, unused_error_message_, ...) EM_FAIL(unused_error_message_) // Does nothing if the message is empty.
#define DETAIL_EM_REFL_INITIALIZER(unused_, ...) __VA_OPT__(= __VA_ARGS__)

// If `...` starts with `_em_verbatim`, returns `a`. Otherwise returns `b`.
// `DETAIL_EM_REFL_IF_VERBATIM(...)(a)(b)`
#define DETAIL_EM_REFL_IF_VERBATIM(...) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REFL_IF_VERBATIM_, __VA_ARGS__)
#define DETAIL_EM_REFL_IF_VERBATIM__em_verbatim ,

// Generates metadata for a class. `seq_` is the list of members, preprocessed with `EM_SEQ_GROUP2(...)`.
// `enable_named_members_` is 0 or 1, whether to generate the member name information.
#define DETAIL_EM_REFL_EMIT_METADATA(seq_, enable_named_members_) \
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
        EM_IF_01(enable_named_members_)( \
            static constexpr ::std::string_view GetMemberName(int _em_i) \
            { \
                static constexpr ::std::array<::std::string_view, num_members> _em_array = { EM_END(DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A seq_) }; \
                return _em_array[::std::size_t(_em_i)]; \
            } \
        )() \
        /* [optional] Lastly, emit any custom traits the user specified via `_em_traits`. */\
        EM_END(DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A seq_) \
    }; \
    /* We're currently using `same_ignoring_cvref<Self>` to reject derived types. */\
    /* Replacing this with `_em_refl_Self` would allow derived types, but that seems to be worse. */\
    friend constexpr _em_refl_Traits _adl_em_refl_Struct(int/*AdlDummy*/, const ::em::Meta::same_ignoring_cvref<_em_refl_Self> auto *) {return {};}

// A loop to count the members.
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(p_type_attrs_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_, EM_REFL_META_KIND(__VA_ARGS__))
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_field +1
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_verbatim
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_traits
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_annotation
// A loop to emit `if constexpr (i == counter) return member; else` for all members, to return references to them.
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP, SF_NULL, 0, seq)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP(n, counter_, p_type_attrs_, name_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_, EM_REFL_META_KIND(name_))(counter_, name_)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_field(counter_, name_) counter_+1, if constexpr (_em_I == counter_) return EM_FWD(_em_self).name_; else
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_verbatim(counter_, ...) counter_
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_traits(counter_, ...) counter_
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_annotation(counter_, ...) counter_
// A loop to emit a list of lists of member types and attributes.
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(p_type_attrs_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_, EM_REFL_META_KIND(__VA_ARGS__))(p_type_attrs_)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_field(p_type_attrs_) , ::em::Refl::MemberInfo<EM_IDENTITY p_type_attrs_>
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_verbatim(...)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_traits(...)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_annotation(...)
// A loop to emit a list of lists of member names.
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(p_type_attrs_, name_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_, EM_REFL_META_KIND(name_))(name_)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_field(name_) #name_,
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_verbatim(...)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_traits(...)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_annotation(...)
// A loop emit custom user-provided code into the traits class.
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(p_type_attrs_, name_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_, EM_REFL_META_KIND(name_))(EM_IDENTITY p_type_attrs_)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_field(...)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_verbatim(...)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_traits(...) __VA_ARGS__
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_annotation(...)

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
