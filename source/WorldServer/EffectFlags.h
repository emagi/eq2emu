/*  
    EQ2Emulator:  Everquest II Server Emulator
    Copyright (C) 2005 - 2026  EQ2EMulator Development Team (http://www.eq2emu.com formerly http://www.eq2emulator.net)

    This file is part of EQ2Emulator.

    EQ2Emulator is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    EQ2Emulator is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with EQ2Emulator.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <shared_mutex>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class LuaSpell;

// ----------------------------- Core bitflags --------------------------------
template <std::size_t NBits>
class BitFlags {
public:
  static constexpr std::size_t kWordBits = 64;
  static constexpr std::size_t kWords    = (NBits + kWordBits - 1) / kWordBits;

  constexpr BitFlags() noexcept : words_{} {}

  // Single-bit ops
  inline void set(std::size_t bit)        noexcept { words_[word_index(bit)] |=  word_mask(bit); }
  inline void reset(std::size_t bit)      noexcept { words_[word_index(bit)] &= ~word_mask(bit); }
  inline void toggle(std::size_t bit)     noexcept { words_[word_index(bit)] ^=  word_mask(bit); }
  inline bool test(std::size_t bit) const noexcept { return (words_[word_index(bit)] & word_mask(bit)) != 0ULL; }

  // Bulk ops
  inline void clear() noexcept {
    for (std::size_t i = 0; i < kWords; ++i) words_[i] = 0ULL;
  }
  inline bool any() const noexcept {
    for (std::size_t i = 0; i < kWords; ++i) if (words_[i]) return true;
    return false;
  }
  inline bool none() const noexcept { return !any(); }

  inline void import_u32(std::uint32_t legacy) noexcept {
    if (!legacy) return;
    const std::size_t lim = (NBits < 32 ? NBits : 32);
    for (std::size_t i = 0; i < lim; ++i)
      if ((legacy >> i) & 1u) set(i);
  }
  inline std::uint32_t export_u32() const noexcept {
    std::uint32_t out = 0;
    const std::size_t lim = (NBits < 32 ? NBits : 32);
    for (std::size_t i = 0; i < lim; ++i)
      if (test(i)) out |= (1u << i);
    return out;
  }
  
  // Import a 64-bit legacy mask (low 64 bits only)
inline void import_u64(std::uint64_t legacy) noexcept {
  if (!legacy) return;
  const std::size_t lim = (NBits < 64 ? NBits : 64);
  for (std::size_t i = 0; i < lim; ++i)
    if ((legacy >> i) & 1ULL) set(i);
}

// Export low 64 bits into a uint64_t
inline std::uint64_t export_u64() const noexcept {
  std::uint64_t out = 0;
  const std::size_t lim = (NBits < 64 ? NBits : 64);
  for (std::size_t i = 0; i < lim; ++i)
    if (test(i)) out |= (1ULL << i);
  return out;
}


  // Set by legacy power-of-two value (find index of the single set bit)
  inline void set_legacy_flag_value(std::uint64_t pow2) noexcept {
    if (!pow2) return;
#if defined(__GNUC__) || defined(__clang__)
    std::size_t bit = static_cast<std::size_t>(__builtin_ctzll(pow2));
#else
    std::size_t bit = 0; while (bit < 64 && ((pow2 >> bit) & 1ULL) == 0ULL) ++bit;
#endif
    if (bit < NBits) set(bit);
  }

  // Direct word access (used by TS wrapper)
  static constexpr std::size_t words_count() noexcept { return kWords; }
  inline std::uint64_t&       word_ref(std::size_t i)       noexcept { return words_[i]; }
  inline const std::uint64_t& word_ref(std::size_t i) const noexcept { return words_[i]; }

private:
  static constexpr std::size_t   word_index(std::size_t bit) noexcept { return bit / kWordBits; }
  static constexpr std::uint64_t word_mask (std::size_t bit) noexcept { return 1ULL << (bit % kWordBits); }

  std::array<std::uint64_t, kWords> words_;
};

// --------------------------- Thread-safe wrapper ----------------------------
template <std::size_t NBits>
class TSBitFlags {
public:
  using Snapshot = BitFlags<NBits>;

  // Single-bit ops
  inline void set(std::size_t bit)        { std::unique_lock lk(m_); bits_.set(bit); }
  inline void reset(std::size_t bit)      { std::unique_lock lk(m_); bits_.reset(bit); }
  inline void toggle(std::size_t bit)     { std::unique_lock lk(m_); bits_.toggle(bit); }
  inline bool test(std::size_t bit) const { std::shared_lock lk(m_); return bits_.test(bit); }

  // Convenience
  inline void add(std::size_t bit)        { set(bit); }
  inline void remove(std::size_t bit)     { reset(bit); }
  inline bool has(std::size_t bit)  const { return test(bit); }

  // Batch ops
  inline void set_many(std::initializer_list<std::size_t> bits) {
    std::unique_lock lk(m_); for (auto b : bits) bits_.set(b);
  }
  inline void reset_many(std::initializer_list<std::size_t> bits) {
    std::unique_lock lk(m_); for (auto b : bits) bits_.reset(b);
  }

  // Clear / any / none
  inline void clear()             { std::unique_lock lk(m_); bits_.clear(); }
  inline bool any() const         { std::shared_lock lk(m_); return bits_.any(); }
  inline bool none() const        { std::shared_lock lk(m_); return bits_.none(); }

  // Legacy helpers
  inline void import_u32(std::uint32_t legacy)     { std::unique_lock lk(m_); bits_.import_u32(legacy); }
  inline std::uint32_t export_u32() const          { std::shared_lock lk(m_); return bits_.export_u32(); }
  inline void import_u64(std::uint64_t legacy)     { std::unique_lock lk(m_); bits_.import_u64(legacy); }
  inline std::uint64_t export_u64() const          { std::shared_lock lk(m_); return bits_.export_u64(); }
  inline void set_legacy_flag_value(std::uint64_t v){ std::unique_lock lk(m_); bits_.set_legacy_flag_value(v); }

  // Snapshots (copy out/in without exposing lock)
  inline Snapshot snapshot() const  { std::shared_lock lk(m_); return bits_; }
  inline void assign_from(const Snapshot& snap) {
    std::unique_lock lk(m_);
    for (std::size_t i = 0; i < Snapshot::words_count(); ++i)
      bits_.word_ref(i) = snap.word_ref(i);
  }

private:
  mutable std::shared_mutex m_;
  Snapshot                  bits_{};
};

// ------------------------- Project-specific typedefs ------------------------
inline constexpr std::size_t kEffectBits = 64; // limited due to DB restrictions at this time
using EffectFlags = TSBitFlags<kEffectBits>;

enum : std::size_t {
  EFFECT_IDX_STUN = 0,
  EFFECT_IDX_ROOT = 1,
  EFFECT_IDX_MEZ  = 2,
  EFFECT_IDX_STIFLE = 3,
  EFFECT_IDX_DAZE = 4,
  EFFECT_IDX_FEAR = 5,
  EFFECT_IDX_SPELLBONUS = 6,
  EFFECT_IDX_SKILLBONUS = 7,
  EFFECT_IDX_STEALTH = 8,
  EFFECT_IDX_INVIS = 9,
  EFFECT_IDX_SNARE = 10,
  EFFECT_IDX_WATERWALK = 11,
  EFFECT_IDX_WATERJUMP = 12,
  EFFECT_IDX_FLIGHT = 13,
  EFFECT_IDX_GLIDE = 14,
  EFFECT_IDX_AOE_IMMUNE = 15,
  EFFECT_IDX_STUN_IMMUNE = 16,
  EFFECT_IDX_MEZ_IMMUNE = 17,
  EFFECT_IDX_DAZE_IMMUNE = 18,
  EFFECT_IDX_ROOT_IMMUNE = 19,
  EFFECT_IDX_STIFLE_IMMUNE = 20,
  EFFECT_IDX_FEAR_IMMUNE = 21,
  EFFECT_IDX_SAFEFALL = 22,
  EFFECT_IDX_ILLUSION = 23,
};



class EffectRegistryPerSpell {
public:
    using Code  = uint8_t; // your int8 codes
    using Value = std::variant<std::monostate, int64_t, double, std::string>;

    explicit EffectRegistryPerSpell(Code max_code_inclusive)
        : max_code_(max_code_inclusive),
          slots_(static_cast<size_t>(max_code_inclusive) + 1) {}

    // --- Membership (writers) -------------------------------------------------

    // Add a spell to a type. Returns true if inserted (false if already present).
    bool Add(Code code, LuaSpell* spell) {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        return s.members.emplace(spell, Entry{spell, std::monostate{}}).second;
    }

    // Add or update (set/replace the per-spell value atomically).
    // Returns true if inserted new, false if updated existing.
    template <typename T>
    bool AddOrUpdate(Code code, LuaSpell* spell, T v) {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        auto [it, inserted] = s.members.emplace(spell, Entry{spell, std::monostate{}});
        it->second.val = to_variant(std::move(v));
        return inserted;
    }

    // Remove a spell from a type. Returns true if erased.
    bool Remove(Code code, LuaSpell* spell) {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        return s.members.erase(spell) > 0;
    }

    // Clear all spells at a type
    void Clear(Code code) {
        if (!in_range(code)) return;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        s.members.clear();
    }

    // --- Queries (readers) ----------------------------------------------------

    // Is there any spell registered at this type?
    bool Has(Code code) const {
        if (!in_range(code)) return false;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        return !s.members.empty();
    }

    // Is this specific spell present under this type?
    bool Contains(Code code, const LuaSpell* spell) const {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        return s.members.find(const_cast<LuaSpell*>(spell)) != s.members.end();
    }

    // How many spells are registered at this type?
    size_t Count(Code code) const {
        if (!in_range(code)) return 0;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        return s.members.size();
    }

    // Snapshot list of spells under a type (copy out while holding a shared lock).
    std::vector<LuaSpell*> Snapshot(Code code) const {
        std::vector<LuaSpell*> out;
        if (!in_range(code)) return out;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        out.reserve(s.members.size());
        for (auto& kv : s.members) out.push_back(kv.first);
        return out;
    }

    // Snapshot of (spell, value) pairs under a type.
    struct SpellWithValue {
        LuaSpell* spell{};
        Value     value{};
    };
    std::vector<SpellWithValue> SnapshotWithValues(Code code) const {
        std::vector<SpellWithValue> out;
        if (!in_range(code)) return out;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        out.reserve(s.members.size());
        for (auto& kv : s.members) out.push_back({kv.second.ptr, kv.second.val});
        return out;
    }

    // --- Per-spell value API --------------------------------------------------

    // Set/replace the value for a specific spell under a type.
    template <typename T>
    bool SetValue(Code code, LuaSpell* spell, T v) {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        auto it = s.members.find(spell);
        if (it == s.members.end()) return false;
        it->second.val = to_variant(std::move(v));
        return true;
    }

    // Clear value for a specific spell under a type (keeps membership).
    bool ClearValue(Code code, LuaSpell* spell) {
        if (!in_range(code) || !spell) return false;
        auto& s = slots_[code];
        std::unique_lock lk(s.mtx);
        auto it = s.members.find(spell);
        if (it == s.members.end()) return false;
        it->second.val = std::monostate{};
        return true;
    }

    // Read value as requested type. nullopt if not set or wrong type.
    template <typename T>
    std::optional<T> GetValue(Code code, const LuaSpell* spell) const {
        if (!in_range(code) || !spell) return std::nullopt;
        auto& s = slots_[code];
        std::shared_lock lk(s.mtx);
        auto it = s.members.find(const_cast<LuaSpell*>(spell));
        if (it == s.members.end()) return std::nullopt;
        if (auto p = std::get_if<T>(&it->second.val)) return *p;
        return std::nullopt;
    }

    // Convenience typed getters
    std::optional<int64_t>     GetInt   (Code code, const LuaSpell* s) const { return GetValue<int64_t>(code, s); }
    std::optional<double>      GetFloat (Code code, const LuaSpell* s) const { return GetValue<double>  (code, s); }
    std::optional<std::string> GetString(Code code, const LuaSpell* s) const { return GetValue<std::string>(code, s); }

    Code MaxCode() const { return max_code_; }

private:
    struct Entry {
        LuaSpell* ptr{};
        Value     val{};
    };
    struct Slot {
        mutable std::shared_mutex mtx;
        std::unordered_map<LuaSpell*, Entry> members; // key = spell*, value = (spell*, variant)
    };

    bool in_range(Code code) const { return code <= max_code_; }

    // normalize to variant
    static Value to_variant(int64_t v)     { return Value{v}; }
    static Value to_variant(int v)         { return Value{static_cast<int64_t>(v)}; }
    static Value to_variant(uint32_t v)    { return Value{static_cast<int64_t>(v)}; }
    static Value to_variant(float v)       { return Value{static_cast<double>(v)}; }
    static Value to_variant(double v)      { return Value{v}; }
    static Value to_variant(const char* s) { return Value{std::string(s ? s : "")}; }
    static Value to_variant(std::string s) { return Value{std::move(s)}; }

    Code max_code_;
    std::vector<Slot> slots_;
};

#define CONTROL_EFFECT_TYPE_MEZ            1
#define CONTROL_EFFECT_TYPE_STIFLE         2
#define CONTROL_EFFECT_TYPE_DAZE           3
#define CONTROL_EFFECT_TYPE_STUN           4
#define CONTROL_EFFECT_TYPE_ROOT           5
#define CONTROL_EFFECT_TYPE_FEAR           6
#define CONTROL_EFFECT_TYPE_WALKUNDERWATER 7
#define CONTROL_EFFECT_TYPE_JUMPUNDERWATER 8
#define CONTROL_EFFECT_TYPE_INVIS          9
#define CONTROL_EFFECT_TYPE_STEALTH        10
#define CONTROL_EFFECT_TYPE_SNARE          11
#define CONTROL_EFFECT_TYPE_FLIGHT         12
#define CONTROL_EFFECT_TYPE_GLIDE          13
#define CONTROL_EFFECT_TYPE_SAFEFALL       14
#define CONTROL_EFFECT_TYPE_ILLUSION       15
#define CONTROL_MAX_EFFECTS                16

#define IMMUNITY_TYPE_MEZ           1
#define IMMUNITY_TYPE_STIFLE        2
#define IMMUNITY_TYPE_DAZE          3
#define IMMUNITY_TYPE_STUN          4
#define IMMUNITY_TYPE_ROOT          5
#define IMMUNITY_TYPE_FEAR          6
#define IMMUNITY_TYPE_AOE           7
#define IMMUNITY_TYPE_TAUNT         8
#define IMMUNITY_TYPE_RIPOSTE       9
#define IMMUNITY_TYPE_STRIKETHROUGH 10
#define IMMUNITY_MAX_TYPES          11

class ControlEffects {
public:
    ControlEffects() : reg_(CONTROL_MAX_EFFECTS - 1) {} // max code = 14

    bool Add(uint8_t type, LuaSpell* s)                                { return reg_.Add(type, s); }
    template <typename T> bool AddOrUpdate(uint8_t t, LuaSpell* s, T v){ return reg_.AddOrUpdate(t, s, std::move(v)); }
    bool Remove(uint8_t type, LuaSpell* s)                              { return reg_.Remove(type, s); }
    void Clear(uint8_t type)                                            { reg_.Clear(type); }

    bool Has(uint8_t type) const                                        { return reg_.Has(type); }
    bool Contains(uint8_t type, const LuaSpell* s) const                { return reg_.Contains(type, s); }
    size_t Count(uint8_t type) const                                    { return reg_.Count(type); }
    std::vector<LuaSpell*> Snapshot(uint8_t type) const                 { return reg_.Snapshot(type); }
    auto SnapshotWithValues(uint8_t type) const                         { return reg_.SnapshotWithValues(type); }

    template <typename T> bool SetValue(uint8_t t, LuaSpell* s, T v)    { return reg_.SetValue(t, s, std::move(v)); }
    bool ClearValue(uint8_t t, LuaSpell* s)                              { return reg_.ClearValue(t, s); }

    template <typename T> std::optional<T> GetValue(uint8_t t, const LuaSpell* s) const { return reg_.GetValue<T>(t, s); }

private:
    EffectRegistryPerSpell reg_;
};

class Immunities {
public:
    Immunities() : reg_(IMMUNITY_MAX_TYPES) {} // max code = 10

    bool Add(uint8_t type, LuaSpell* s)                                { return reg_.Add(type, s); }
    template <typename T> bool AddOrUpdate(uint8_t t, LuaSpell* s, T v){ return reg_.AddOrUpdate(t, s, std::move(v)); }
    bool Remove(uint8_t type, LuaSpell* s)                              { return reg_.Remove(type, s); }
    void Clear(uint8_t type)                                            { reg_.Clear(type); }

    bool Has(uint8_t type) const                                        { return reg_.Has(type); }
    bool Contains(uint8_t type, const LuaSpell* s) const                { return reg_.Contains(type, s); }
    size_t Count(uint8_t type) const                                    { return reg_.Count(type); }
    std::vector<LuaSpell*> Snapshot(uint8_t type) const                 { return reg_.Snapshot(type); }
    auto SnapshotWithValues(uint8_t type) const                         { return reg_.SnapshotWithValues(type); }

    template <typename T> bool SetValue(uint8_t t, LuaSpell* s, T v)    { return reg_.SetValue(t, s, std::move(v)); }
    bool ClearValue(uint8_t t, LuaSpell* s)                              { return reg_.ClearValue(t, s); }

    template <typename T> std::optional<T> GetValue(uint8_t t, const LuaSpell* s) const { return reg_.GetValue<T>(t, s); }

private:
    EffectRegistryPerSpell reg_;
};