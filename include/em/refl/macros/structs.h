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
#include "em/meta/common.h" // IWYU pragma: keep, used in the macros.
#include "em/meta/lists.h" // IWYU pragma: keep, used in the macros.
#include "em/refl/common.h"

#include <array> // IWYU pragma: keep, used in the macros.
#include <cstddef> // IWYU pragma: keep, used in the macros.
#include <string_view> // IWYU pragma: keep, used in the macros.
#include <type_traits> // IWYU pragma: keep, used in the macros.


namespace em::Refl::Structs
{
    namespace detail::Macros
    {
        // Given a `type, attributes...` list (as accepted by `EM_REFL`), returns the `type` from it.
        template <typename Type, Attribute ...Attrs>
        using MemberType = Type;

        template <typename Derived>
        constexpr void _adl_em_InheritanceHook(void *) {} // Dummy ADL target.
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

// Will preprocess the entire input sequence of `EM_REFL(...)` (including verbatim blocks and annotations, but excluding control statements),
//   using those macros. Those are fed as parameters to `SF_FOR_EACH`.
// On each iteration you'll receive one of the following entry kinds. You can emit it back as is, if you want to leave it unchanged.
// You can emit more than one entry if you want, or nothing at all.
// The input arrives without parentheses, so you must use multiple arguments. Your output must be parenthesized, add `(...)` yourself.
// If you want to keep the current entry as is, you must emit it unchanged, but in parentheses.
// There are following entry kinds. Notice that the first element of each is a literal tag that you can use for dispatch.
//     (field, (type_ [,attrs_...]), name_ [,init_...])    // A field declaration.
//     (verbatim, target_, tag_, metadata_, text_...)      // A verbatim text block.
//                                                            Here `target_` is one of:
//                                                              `body` - emitted in the class body, between the data memebrs.
//                                                              `traits` - emitted in the internal traits class that's generated for this class.
//     (annotation, category_, error_if_unused_, data_...) // An annotation, for use by external mixins. We ignore them, other than optionally
//                                                              erroring if they are unused.
//                                                            `category_` is a single word for dispatch, can be anything.
//                                                            `error_if_unused_` is either empty or a string literal. If it's a string, it indicates that
//                                                              this annotation is unused, and we'll error at preprocessing time with this message.
//                                                              Set it to empty in your loop to mark the annotation as used.
//                                                              It can also be set to empty from the very beginning, if the annotation is intended
//                                                                to be silently ignored if unused.
// NOTE: When writing nested loops, you must respect the `n` parameter as usual!
// NOTE: Use `_em_Self` to refer to the enclosing class.
// Debugging hint: you can temporarily comment out `DETAIL_EM_REFL_3` below to see the result of the preprocessing pass when expanding `EM_REFL()`.
#define EM_REFL_PREPROCESS_LOW(body_, step_, final_, data_) (,_em_preprocess, body_, step_, final_, data_)
// ]

// Things that can appear between the members of `EM_REFL()`: [

// Access modifiers for `EM_REFL(...)`.
#define EM_PUBLIC    EM_REFL_VERBATIM_LOW(body, access, public   , public:   )
#define EM_PRIVATE   EM_REFL_VERBATIM_LOW(body, access, private  , private:  )
#define EM_PROTECTED EM_REFL_VERBATIM_LOW(body, access, protected, protected:)
// Pastes a piece of code verbatim in `EM_REFL(...)`.
#define EM_VERBATIM(...) EM_REFL_VERBATIM_LOW(body, user,, __VA_ARGS__)

// An extended version of `EM_VERBATIM()`.
// `target_` is where this text should be pasted. One of:
//   * `body` (class body).
//   * `traits` (the emitted traits class).
// `tag_` is an arbitrary single-word tag.
// `metadata_` is arbitrary data, not necessarily single-word.
// `...` is the text that's inserted.
// Remember that you can use `_em_Self` to refer to the enclosing class.
// Feel free to asssemble the underlying sequence directly, without this macro, if you e.g. want to support syntax like `FOO(a...)(b...)`,
//   which would be a bit trick to propagate to a macro like this one.
#define EM_REFL_VERBATIM_LOW(target_, tag_, metadata_, .../*text*/) (verbatim, target_, tag_, metadata_, __VA_ARGS__)(_em_meta)
// Inserts a custom annotation in `EM_REFL()`.
// `category_` is a single-word ID that roughly explains what this is.
// `error_if_unused_` is either empty or a string literal. If not empty, `EM_REFL()` will emit this error message if no one removes it from
//   this annotation via `EM_REFL_PREPROCESS_LOW()`, which would indicate that the annotation is successfully consumed.
// `...` can be anything, we don't access it directly.
// Feel free to asssemble the underlying sequence directly, without this macro, if you e.g. want to support syntax like `FOO(a...)(b...)`,
//   which would be a bit trick to propagate to a macro like this one.
#define EM_REFL_ANNOTATE_LOW(category_, error_if_unused_, .../*data*/) (annotation, category_, error_if_unused_, __VA_ARGS__)(_em_meta)

// ]

// Additional helpers: [

// An inheritance hook. You can paste this in class body, e.g. using `verbatim` or manually.
// This runs at program startup for every `EM_REFL()` class derived from this.
// `unique_` must be a unique string per class. Only matters if you have multiple hooks per class. Can be empty in one of them. Can start with a digit.
// `cond_` is a condition in terms of `_em_Self` and `_em_Derived`, typically parenthesized, that checks if this hook should be enabled.
//   This should probably check either `std::derived_from` or `std::is_base_of`, and possibly check for equality between the two types,
//   if you want to reject that, and possibly abstract-ness of `_em_Derived`, etc.
//   It's IMPORTANT to check at least either `derived_from` or `is_base_of`, or you will incorrectly get non-derived classes in there too sometimes.
// `...` is the hook body that runs on program startup. Use `_em_Self` and `_em_Derived` in it. It can have multiple statements.
//   If `...` is empty, the entire macro expands to nothing.
// The inheritance hook itself. This is only emitted if the contents are non-empty.
// Remember that you can use `_em_Self` to refer to the enclosing class.
#define EM_REFL_INHERITANCE_HOOK(unique_, cond_, ...) \
    __VA_OPT__(\
        /* Would making this a lambda a work? A function is generally more reliable (ensures the enclosing class is complete in the body), */\
        /* so it's easier to keep this a function. */\
        template <typename _em_Derived/*this is a user-facing name*/> static ::std::nullptr_t EM_CAT(_em_InheritanceHook_Func, unique_)() {__VA_ARGS__ return nullptr;} \
        template <typename _em_Derived> inline static const ::std::nullptr_t EM_CAT(_em_InheritanceHook_Var, unique_) = EM_CAT(_em_InheritanceHook_Func, unique_)<_em_Derived>(); \
        /* Now an ADL target. */\
        /* This function intentionally doesn't have `unique_` appended to its name. Otherwise we wouldn't be able to call it, and it's not needed anyway. */\
        template <\
            typename _em_Derived, \
            /* Confirm that this is inheritance. We're using `is_base_of` instead of `derived_from` here, just in case. */\
            /* E.g. in case the base is a tag we can inherit multiple times from? The user should check `derived_from` themselves, */\
            /* or accept an error on ambiguity. */\
            ::std::enable_if_t<cond_, ::std::nullptr_t> = nullptr, \
            /* Instantiate the variable. */\
            auto = &EM_CAT(_em_InheritanceHook_Var, unique_)<_em_Derived>, \
            /* This condition is always false. */\
            /* The variable is mentioned here AGAIN to make different hooks in the same class not collide with each (to make the function types unique), */\
            /* but this isn't enough to instantiate the variable, so we also keep another mention of it above. */\
            ::std::enable_if_t<::em::Meta::always_false<_em_Derived, ::em::Meta::ValueTag<&EM_CAT(_em_InheritanceHook_Var, unique_)<_em_Derived>>>, std::nullptr_t> = nullptr \
        > \
        friend constexpr void _adl_em_InheritanceHook(void *) {} \
    )

// ]


// --- Internals:

// This is used to separate the leading control statements from the rest of the `EM_REFL(...)` input sequence.
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

// `seq` is the argument of `EM_REFL(...)` macro, minus the control statements that are already removed by `DETAIL_EM_REFL_SPLIT_LIST()`,
//   and already preprocessed with `EM_SEQ_GROUP2()`, which converts `(a...)(b...)` to `((a...), b...)`.
// This function converts the format of that to something more convenient for internal use.
// Fields of the form `(type_ [,attrs_...])(name_ [,init_...])` become `(field, (type_ [,attrs_...]), name_ [,init_...])` (where `field` is inserted literally,
//   it's not a placeholder).
// Everything else is left as is, minus the `(_em_meta)` suffix. E.g. verbatim blocks become `(verbatim, target_, tag_ metadata_, text...)`,
//   and similarly for annotations.
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT(seq_) EM_END(DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_A seq_)
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_A(...) DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_BODY(__VA_ARGS__) DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_B
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_B(...) DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_BODY(__VA_ARGS__) DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_A
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_A_END
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_LOOP_B_END
// Here `...` can't be split into `name_, ...`, because we ahve to handle `name_` and `name_,` differently, see `DETAIL_EM_REFL_EMIT_MEMBERS()` below.
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_BODY(parens_, ...) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_BODY_CHECK, __VA_ARGS__)(parens_)((field, parens_, __VA_ARGS__))
#define DETAIL_EM_REFL_ADJUST_SEQ_FORMAT_BODY_CHECK_em_meta ,

// The implementaiton of `EM_REFL(...)`.
#define DETAIL_EM_REFL(...) DETAIL_EM_REFL_2(__VA_ARGS__)
#define DETAIL_EM_REFL_2(control_, seq_) DETAIL_EM_REFL_3(SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_CONTROL_STEP, SF_STATE_EXPAND, (/*elems:*/DETAIL_EM_REFL_ADJUST_SEQ_FORMAT(EM_SEQ_GROUP2(seq_)), /*enable member names:*/1), control_))
#define DETAIL_EM_REFL_CONTROL_STEP(n, d, empty_, command_, ...) DETAIL_EM_REFL_CONTROL_STEP_1(command_, EM_IDENTITY d __VA_OPT__(,) __VA_ARGS__)
#define DETAIL_EM_REFL_CONTROL_STEP_1(command_, ...) EM_CAT(DETAIL_EM_REFL_CONTROL_STEP_1_, command_)(__VA_ARGS__)
#define DETAIL_EM_REFL_CONTROL_STEP_1__em_unnamed_members(seq_, enable_member_names_) (seq_, EM_IF_01(enable_member_names_)(0)(EM_FAIL("Duplicate `EM_UNNAMED_MEMEBRS`")))
#define DETAIL_EM_REFL_CONTROL_STEP_1__em_preprocess(seq_, enable_member_names_, body_, step_, final_, data_) (SF_FOR_EACH0(body_, step_, final_, data_, seq_), enable_member_names_)
#define DETAIL_EM_REFL_3(...) DETAIL_EM_REFL_4(__VA_ARGS__)
// Here we emit most of the metadata before the members, because at the end of members we can have some custom code injected by `EM_REFL_PREPROCESS_LOW()`,
//   which might expect the metadata to already exist.
// Having metadata before the members isn't an issue at all, because all the members are only named inside of functions, which can happen before they are declared.
// After after the members we add the inheritance hooks, in case the code in them depends on those members already existing. I didn't test how much this affects this, but it's clearly better to keep it this way.
#define DETAIL_EM_REFL_4(seq_, enable_member_names_) \
    DETAIL_EM_REFL_EMIT_METADATA_PRE(seq_, enable_member_names_) \
    DETAIL_EM_REFL_EMIT_MEMBERS(seq_) \
    DETAIL_EM_REFL_EMIT_METADATA_POST(seq_)

// Takes a member list (preprocessed with `EM_SEQ_GROUP2(...)`) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq_) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_KIND_, kind_)(__VA_ARGS__)
// Note that this is written in a particular way, to handle `name` and `name,` differently. The former gets zeroed via `{}`, while the latter is left uninitialized. The user can use the latter for types that are not default-constructible, which they plan to initialize in the member init list.
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_KIND_field(p_type_attrs_, .../*init*/) ::em::Refl::Structs::detail::Macros::MemberType<EM_IDENTITY p_type_attrs_> EM_VA_FIRST(__VA_ARGS__) EM_IF_COMMA(__VA_ARGS__)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_FIELD_INIT(__VA_ARGS__))({});
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_FIELD_INIT(unused, ...) __VA_OPT__(= __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_KIND_verbatim(target_, tag_, metadata_, .../*text*/) EM_CAT(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM_, target_)(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM_body(...) __VA_ARGS__
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM_traits(...)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_KIND_annotation(category_, error_if_unused_, .../*data*/) EM_FAIL(error_if_unused_)


// Generates metadata for a class, the part of it that appears before the data members.
#define DETAIL_EM_REFL_EMIT_METADATA_PRE(seq_, enable_member_names_) \
    /* Typedef the enclosing class. This is intentionally not prefixed with `refl`, because it can appear in the user-written inheritance hooks, */\
    /* and I don't want to spell `refl` every time. The reflection is kinda privileged, so I don't see a problem with not prefixing it specifically */\
    /* in the reflection. */\
    EM_TYPEDEF_ENCLOSING_CLASS(_em_Self) \
    \
    /* Detect this as a base class. */\
    EM_DETECTABLE_BASE((::em::Refl::detail::StructBasesTag), (_em_Self)) \
    \
    struct _em_refl_Traits \
    { \
        /* Member count. */\
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
        EM_IF_01(enable_member_names_)( \
            static constexpr ::std::string_view GetMemberName(int _em_i) \
            { \
                static constexpr ::std::array<::std::string_view, num_members> _em_array = { EM_END(DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A seq_) }; \
                return _em_array[::std::size_t(_em_i)]; \
            } \
        )() \
        /* --- The stuff below is not customization points, don't add it to your own traits. */\
        /* Emit any custom traits the user specified via `EM_REFL_VERBATIM_LOW()`. */\
        EM_END(DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A seq_) \
    }; \
    /* This function is used to obtain traits for a class. */\
    /* We're currently using `same_ignoring_cvref<Self>` to reject derived types. */\
    /* Replacing this with `_em_Self` would allow derived types, but that seems to be worse. */\
    friend constexpr _em_refl_Traits _adl_em_refl_Struct(int/*AdlDummy*/, const ::em::Meta::same_ignoring_cvref<_em_Self> auto *) {return {};}

// Generates metadata for a class, the part of it that appears after the data members.
#define DETAIL_EM_REFL_EMIT_METADATA_POST(seq_) \
    /* Poke the inheritance hook from any of the base classes. This is at the end just in case.*/\
    DETAIL_EM_REFL_TRIGGER_INHERITANCE_HOOK

// A loop to count the members.
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_, kind_)
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_field +1
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_verbatim
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY_annotation
// A loop to emit `if constexpr (i == counter) return member; else` for all members, to return references to them.
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP, SF_NULL, 0, seq)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP(n, counter_, kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_, kind_)(counter_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_field(counter_, p_type_attrs_, name_, ...) counter_+1, if constexpr (_em_I == counter_) return EM_FWD(_em_self).name_; else
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_verbatim(counter_, ...) counter_
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP_annotation(counter_, ...) counter_
// A loop to emit a list of lists of member types and attributes.
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY(kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_, kind_)(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_field(p_type_attrs_, ...) , ::em::Refl::MemberInfo<EM_IDENTITY p_type_attrs_>
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_verbatim(...)
#define DETAIL_EM_REFL_EMIT_METADATA_ATTRS_LOOP_BODY_annotation(...)
// A loop to emit a list of lists of member names.
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY(kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_, kind_)(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_field(p_type_attr_, name_, ...) #name_,
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_verbatim(...)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_traits(...)
#define DETAIL_EM_REFL_EMIT_METADATA_NAMES_LOOP_BODY_annotation(...)
// A loop emit custom user-provided code into the traits class.
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY(kind_, ...) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_KIND_, kind_)(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_KIND_field(...)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_KIND_verbatim(target_, tag_, metadata_, .../*text*/) EM_CAT(DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_VERBATIM_, target_)(__VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_VERBATIM_body(...)
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_VERBATIM_traits(...) __VA_ARGS__
#define DETAIL_EM_REFL_EMIT_METADATA_TRAITS_LOOP_BODY_KIND_annotation(...)

// Trigger the inheritance hook in the base classes, and in this class itself.
#define DETAIL_EM_REFL_TRIGGER_INHERITANCE_HOOK \
    static void _em_TriggerInheritanceHook() \
    { \
        using ::em::Refl::Structs::detail::Macros::_adl_em_InheritanceHook; \
        _adl_em_InheritanceHook<_em_Self>((_em_Self *)nullptr);\
    } \
    static constexpr ::std::integral_constant<void (*)(), _em_TriggerInheritanceHook> _em_TriggerInheritanceHookVar{};
