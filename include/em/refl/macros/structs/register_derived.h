#pragma once

#include "em/macros/meta/common.h"
#include "em/macros/meta/if_else.h"
#include "em/refl/macros/structs/core.h"

// This is a control statement that can be added at the beginning of `EM_REFL(...)` to make
//   every class derived from this (including indirectly) register itself.
// Combine this with `EM_STATIC_VIRTUAL()` (see below) to type-erase various operations on the derived classes.
#define EM_REGISTER_DERIVED EM_REFL_PREPROCESS_LOW(SF_NULL, DETAIL_EM_REGISTER_DERIVED_STEP, DETAIL_EM_REGISTER_DERIVED_FINAL, /*empty*/)

// Usage: `EM_STATIC_VIRTUAL(FuncName, (params...) -> ReturnType)(return whatever<_em_T>();)`.
// In the function body (last argument), `_em_T` is the type of the derived class.
// Implementation detail: Here we aren't using `EM_REFL_ANNOTATE_LOW()` to support this call syntax, but the end result is equivalent.
#define EM_STATIC_VIRTUAL(name_, ...) (em_static_virtual, "`EM_STATIC_VIRTUAL()` requires `EM_REGISTER_DERIVED`", name_, (__VA_ARGS__), DETAIL_EM_STATIC_VIRTUAL_BODY
#define DETAIL_EM_STATIC_VIRTUAL_BODY(...) __VA_ARGS__)(_em_annotation)


#define DETAIL_EM_REGISTER_DERIVED_STEP(n, d, p_type_attrs_, ...) EM_CAT(DETAIL_EM_REGISTER_DERIVED_STEP_, EM_REFL_META_KIND(__VA_ARGS__))(d, p_type_attrs_, __VA_ARGS__)
#define DETAIL_EM_REGISTER_DERIVED_STEP_field(d, ...) d,(__VA_ARGS__) // Keep.
#define DETAIL_EM_REGISTER_DERIVED_STEP_verbatim(d, ...) d,(__VA_ARGS__) // Keep.
#define DETAIL_EM_REGISTER_DERIVED_STEP_traits(d, ...) d,(__VA_ARGS__) // Keep.
#define DETAIL_EM_REGISTER_DERIVED_STEP_annotation(d, p_data_, ...) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REGISTER_DERIVED_CHECK_, EM_IDENTITY p_data_)(DETAIL_EM_REGISTER_DERIVED_STEP_annotation_1(d, EM_IDENTITY p_data_))(d,(p_data_, __VA_ARGS__))
#define DETAIL_EM_REGISTER_DERIVED_STEP_annotation_1(...) DETAIL_EM_REGISTER_DERIVED_STEP_annotation_2(__VA_ARGS__)
#define DETAIL_EM_REGISTER_DERIVED_STEP_annotation_2(d, category_, error_if_unused_, ...) d(__VA_ARGS__), ((category_,, __VA_ARGS__), _em_annotation) // Store this annotation to data, but also emit it back without the unused flag.
#define DETAIL_EM_REGISTER_DERIVED_CHECK_em_static_virtual ,

#define DETAIL_EM_REGISTER_DERIVED_FINAL(n, .../*d*/) __VA_OPT__(DETAIL_EM_REGISTER_DERIVED_FINAL_1(n, __VA_ARGS__))
#define DETAIL_EM_REGISTER_DERIVED_FINAL_1(n, d) \
    /* This block goes straight to the traits, see `_em_traits` at the end. */\
    (( \
        struct _em_RegisterDerivedBase \
        { \
            virtual ~_em_RegisterDerivedBase() = default; \
            SF_CAT(SF_FOR_EACH, n)(DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE, SF_NULL, SF_NULL,, d) \
        }; \
        template <typename _em_T> \
        struct _em_RegisterDerivedImpl : _em_RegisterDerivedBase \
        { \
            SF_CAT(SF_FOR_EACH, n)(DETAIL_EM_REGISTER_DERIVED_BODY_IMPL, SF_NULL, SF_NULL,, d) \
        }; \
    ), _em_traits)

#define DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE(n, d, name_, p_params_ret_, .../*body*/) \
    /* Using a trailing return type to avoid having to wrap it in `std::type_identity_t`. */\
    [[nodiscard]] virtual auto name_ DETAIL_EM_REGISTER_DERIVED_INJECT_CONST EM_IDENTITY p_params_ret_ = 0;

#define DETAIL_EM_REGISTER_DERIVED_BODY_IMPL(n, d, name_, p_params_ret_, .../*body*/) \
    /* Using a trailing return type to avoid having to wrap it in `std::type_identity_t`. */\
    [[nodiscard]] auto name_ DETAIL_EM_REGISTER_DERIVED_INJECT_CONST EM_IDENTITY p_params_ret_ override {__VA_ARGS__}

#define DETAIL_EM_REGISTER_DERIVED_INJECT_CONST(...) (__VA_ARGS__) const

#define DETAIL_EM_REGISTER_DERIVED_PARAM_DECLS(n, params_) SF_CAT(SF_FOR_EACH, n)(SF_NULL, DETAIL_EM_REGISTER_DERIVED_BODY_PARAM_DECL, SF_NULL, EM_EMPTY, params_)
#define DETAIL_EM_REGISTER_DERIVED_STEP_PARAM_DECL(n, d, name_, .../*type*/) EM_COMMA, d(,) std::type_identity_t<__VA_ARGS__> name_
