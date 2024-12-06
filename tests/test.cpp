#include <em/macros/meta/common.h>
#include <em/macros/meta/if_else.h>
#include <em/macros/meta/ranges.h>
#include <em/macros/meta/sequence_for.h>
#include <em/macros/utils/forward.h>
#include <em/meta/packs.h>
#include <em/meta/tags.h>


#define EM_PUBLIC    EM_VERBATIM_LOW(access, public   , public:   )
#define EM_PRIVATE   EM_VERBATIM_LOW(access, private  , private:  )
#define EM_PROTECTED EM_VERBATIM_LOW(access, protected, protected:)
#define EM_VERBATIM(...) EM_VERBATIM_LOW(user,, __VA_ARGS__)

// For writing extensions. `...` gets pasted in the class verbatim.
// `category_` is a single-word ID that roughly explains what this is. `data_` can be anything.
// `...`, if specified, is inserted literally into the class, inbetween the members.
#define EM_VERBATIM_LOW(category_, data_, .../*verbatim*/) (_em_verbatim)(category_,data_,__VA_ARGS__)

// `EM_STRUCT(name)(...)` is a shorthand for `struct name {EM_REFL(...)};`.
#define EM_STRUCT(.../*name*/) struct __VA_ARGS__ DETAIL_EM_STRUCT_BODY
// `EM_CLASS(name)(...)` is a shorthand for `struct class {EM_REFL(...)};`.
#define EM_CLASS(.../*name*/) class __VA_ARGS__ DETAIL_EM_STRUCT_BODY
#define DETAIL_EM_STRUCT_BODY(...) { EM_REFL(__VA_ARGS__) };



// Takes a member list (preprocessed with `EM_SEQ_GROUP2(...)`) and outputs the member declarations for it.
#define DETAIL_EM_REFL_EMIT_MEMBERS(seq_) EM_END(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A seq_)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B(...) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY(p_type_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM)(DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL)(p_type_, __VA_ARGS__)
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_DECL(p_type_, name_, .../*init_*/) ::std::type_identity_t<EM_IDENTITY p_type_> name_ EM_IF_NONEMPTY(__VA_ARGS__)(=__VA_ARGS__)({});
#define DETAIL_EM_REFL_EMIT_MEMBERS_LOOP_BODY_VERBATIM(p_type_, category_, data_, .../*text_*/) __VA_ARGS__

// If `p_type_` is `_em_verbatim`, returns `a`. Otherwise returns `b`.
// `DETAIL_EM_REFL_IF_VERBATIM(p_type_)(a)(b)`
#define DETAIL_EM_REFL_IF_VERBATIM(p_type_) EM_IF_CAT_ADDS_COMMA(DETAIL_EM_REFL_IF_VERBATIM_, EM_IDENTITY p_type_)
#define DETAIL_EM_REFL_IF_VERBATIM__em_verbatim ,

// Generates metadata for a class. `seq_` is the list of members, preprocessed with `EM_SEQ_GROUP2(...)`.
#define DETAIL_EM_REFL_EMIT_METADATA(seq_) \
    /* Member count. */\
    [[nodiscard]] constexpr ::std::size_t _em_refl_NumMembers = EM_END(DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A seq_); \
    /* A getter for the members. */\
    [[nodiscard]] constexpr ::std::size_t _em_refl_GetMember([[maybe_unused]] this auto &&_em_self) \
    { \
        DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq_) \
        static_assert(::em::meta::always_false<decltype(_em_self), decltype(_em_i_tag)>, "Member index is out of range."); \
    }
// A loop to count the members.
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B(...) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(__VA_ARGS__) DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_A_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_B_END
#define DETAIL_EM_REFL_EMIT_METADATA_COUNT_LOOP_BODY(p_type_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_)()(+1)
// A loop to emit `if constexpr (i == counter) return member; else` for all members.
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP(seq) SF_FOR_EACH(SF_NULL, DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP, SF_NULL, 0, seq)
#define DETAIL_EM_REFL_EMIT_METADATA_GETMEMBER_LOOP_STEP(n, counter_, p_type_, name_, ...) DETAIL_EM_REFL_IF_VERBATIM(p_type_)(counter_)(counter_+1, if constexpr (_em_I == counter_) return EM_FWD(_em_self).name_; else)

#define DETAIL_EM_REFL_SPLIT_LIST(seq_) EM_END(DETAIL_EM_REFL_SPLIT_LIST_LOOP_A seq_)
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A(...) EM_IF_EMPTY(EM_VA_FIRST(__VA_ARGS__))((__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_B)(,(__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_C)
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B(...) EM_IF_EMPTY(EM_VA_FIRST(__VA_ARGS__))((__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_A)(,(__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_C)
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_C(...) (__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_D
#define DETAIL_EM_REFL_SPLIT_LIST_LOOP_D(...) (__VA_ARGS__) DETAIL_EM_REFL_SPLIT_LIST_LOOP_C
// #define DETAIL_EM_REFL_SPLIT_LIST_LOOP_A_END
// #define DETAIL_EM_REFL_SPLIT_LIST_LOOP_B_END
// #define DETAIL_EM_REFL_SPLIT_LIST_LOOP_C_END
// #define DETAIL_EM_REFL_SPLIT_LIST_LOOP_D_END

EM_STR(DETAIL_EM_REFL_SPLIT_LIST((,a)(,a)))

DETAIL_EM_REFL_EMIT_METADATA(DETAIL_EM_REFL_PREPROCESS_NORMAL(
    (int)(x)
EM_PUBLIC
    (map<int,float>)(y,{1,2})
),)
