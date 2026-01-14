#ifndef ENGINE_UTIL_SERIALIZATION_H
#define ENGINE_UTIL_SERIALIZATION_H

#include "util/BasicLog.h"
#include "util/Reflection.h"
#include "util/TemplateConcepts.h"
#include <memory> // Needed for std::shared_ptr

namespace Engine {
namespace Util {

    /**
     * @brief A type for a compiler annotation to skip a property during serialization
     * Use like this:
     * class X {
     * public:
     *  [[=Util::SkipSerialization]] int _propertyToSkip;
     * }
     * 
     */
    constexpr struct SkipSerializationType {} SkipSerialization;

    template<class T>
    concept SpecialSerialization = FundamentalType<T> || PointerType<T> || (IterableSTL<T> && InsertableSTL<T>)
        || IsType<T, std::string> || IsTemplatedType<T, ^^std::shared_ptr> || IsTemplatedType<T, ^^std::unique_ptr> || IsTemplatedType<T, ^^std::atomic>
        || IsTemplatedType<T, ^^std::queue> || IsTemplatedType<T, ^^std::priority_queue> || IsTemplatedType<T, ^^std::stack>
        || ArrayType<T> || IsTemplatedType<T, ^^std::array>
        || IsTemplatedType<T, ^^std::chrono::time_point> || IsTemplatedType<T, ^^std::chrono::duration>
        || IsTemplatedType<T, ^^std::pair>;
    
    // All the std library types use special allocations, so I do not want to accept types that are not implemented as they will garantuee bugs
    template<class T>
    concept NotImplementedSerialization = IsInNamespace<^^T, ^^std>() && !SpecialSerialization<T>;

    /**
     * @brief A helper class to make implementing custom serialization easier
     * Uses CRTP for almost virtual templated functions
     * You need to implement the following methods:
     * ```
     * class CustomSerializer : public Util::Serializer<CustomSerializer> {
     *      void StartClass(const size_t amountMembers, const std::string_view name) override {}
     *      void EndClass() override {}
     *
     *      // The template T will be the type of the container (for example std::vector<uint8_t>)
     *      // You can use reflection/concepts to distinguis different containers
     *      template<class T>
     *      void StartSTLContainer(const size_t size, const std::string_view name) {}
     *      // Template T is the same as for StartSTLContainer
     *      template<class T>
     *      void EndSTLContainer() {}
     * 
     *      // Is called with (u)int8_t till (u)int64_t, float, double and long double
     *      template<FundamentalType T>
     *      void AddVariable(const T& t, const std::string_view name) {}
     * 
     *      void AddVariable(const std::string& t, const std::string_view name) override {}
     * }
     * ```
     * 
     * @tparam Derived The class that derives and implements all the methods called by Serializer (see above)
     */
    template<class Derived>
    class Serializer {
    public:
        
    protected:
        virtual void StartClass(const size_t amountMembers, const std::string_view name) {}
        virtual void EndClass() {}

        template<class T>
        void StartSTLContainer(const size_t size, const std::string_view name) {}
        template<class T>
        void EndSTLContainer() {}

        template<FundamentalType T>
        void AddVariable(const T t, const std::string_view name) {}
        virtual void AddVariable(const std::string& t, const std::string_view name) {}

        /**
         * @brief Counts the publicly accesible members of a class (without the SkipSerialization annotation)
         * 
         * @tparam T The type
         * @return The amount of member variables 
         */
        template<class T>
        static consteval size_t CountMembers() {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            size_t count = 0;
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                count++;
            }
            return count;
        }

        /**
         * @brief Will serialize all the members of t accesible to this
         * 
         * @tparam T Object type to serialize
         * @param serializer The serializer specialization to use
         * @param t The object to serialize using the serializer param
         */
        template<class T> requires (!SpecialSerialization<T> && !NotImplementedSerialization<T>)
        void Serialize(const T& t, const std::string_view name="") {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = CountMembers<T>();
            if(amountMembers == 0) WARNING("[Util::Serializer] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members?")
            
            StartClass(amountMembers, name);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                else Serialize(t.[:memberTypeInfo:], std::meta::identifier_of(memberTypeInfo));
            }
            EndClass();
        }

        template<NotImplementedSerialization T>
        void Serialize(const T& t, const std::string_view name) {
            static_assert(false, "[Util::Deserializer] STD library class not yet implemented");
        }

        template<FundamentalType T> requires (!PointerType<T>)
        void Serialize(const T& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template AddVariable<std::remove_cv_t<T>>(t, name);
        }
        void Serialize(const std::string& t, const std::string_view name="") {
            static_cast<Derived*>(this)->AddVariable(t, name);
        }
        template<class T>
        void Serialize(const T*& t, const std::string_view name="") {
            Serialize(*t, name);
        }
        template<class T>
        void Serialize(const std::shared_ptr<T>& t, const std::string_view name="") {
            Serialize(*t, name);
        }
        template<class T>
        void Serialize(const std::unique_ptr<T>& t, const std::string_view name="") {
            Serialize(*t, name);
        }
        template<class T>
        void Serialize(const std::atomic<T>& t, const std::string_view name="") {
            Serialize(t.load(), name);
        }

        template<class T> requires IterableSTL<T> && InsertableSTL<T>
        void Serialize(const T& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<T>(t.size(), name);
            
            for(const auto& elem : t) {
                Serialize(elem, "STL-Container-Element");
            }

            static_cast<Derived*>(this)->template EndSTLContainer<T>();
        }
        template<class T, class Allocator>
        void Serialize(const std::forward_list<T, Allocator>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::forward_list<T, Allocator>>(t.size(), name);
            
            for(const auto& elem : t) {
                Serialize(elem, "STL-Forward-List-Element");
            }

            static_cast<Derived*>(this)->template EndSTLContainer<std::forward_list<T, Allocator>>();
        }
        template<class T, class Container>
        void Serialize(const std::queue<T, Container>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::queue<T, Container>>(t.size(), name);
            // Because queues are stupid, we need to first make a copy to access the actual data
            std::queue<T, Container> copy = t;
            while(copy.size()) {
                Serialize(t.front(), "STL-Queue-Element");
                t.pop();
            }
            static_cast<Derived*>(this)->template EndSTLContainer<std::queue<T, Container>>();
        }
        template<class T, class Container, class Compare>
        void Serialize(const std::priority_queue<T, Container, Compare>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::priority_queue<T, Container, Compare>>(t.size(), name);
            // Because queues are stupid, we need to first make a copy to access the actual data
            std::priority_queue<T, Container, Compare> copy = t;
            while(copy.size()) {
                Serialize(t.top(), "STL-Priority-Queue-Element");
                t.pop();
            }
            static_cast<Derived*>(this)->template EndSTLContainer<std::priority_queue<T, Container, Compare>>();
        }
        template<class T, class Container>
        void Serialize(const std::stack<T, Container>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::stack<T, Container>>(t.size(), name);
            // Because stacks are stupid, we need to first make a copy to access the actual data
            std::stack<T, Container> copy = t;
            while(copy.size()) {
                Serialize(t.top(), "STL-Stack-Element");
                t.pop();
            }
            static_cast<Derived*>(this)->template EndSTLContainer<std::stack<T, Container>>();
        }
        template<class T, size_t N>
        void Serialize(const std::array<T, N>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::array<T, N>>(t.size(), name);
            
            for(const auto& elem : t) {
                Serialize(elem, "STL-Array-Element");
            }

            static_cast<Derived*>(this)->template EndSTLContainer<std::array<T, N>>();
        }
        template<class T, size_t N>
        void Serialize(const T (&t)[N], const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::array<T, N>>(N, name);
            
            for(const auto& elem : t) {
                Serialize(elem, "Array-Element");
            }

            static_cast<Derived*>(this)->template EndSTLContainer<std::array<T, N>>();
        }

        template<class Clock, class Duration>
        void Serialize(const std::chrono::time_point<Clock, Duration>& t, const std::string_view name="") {
            if constexpr(IsType<Clock, std::chrono::steady_clock>) {
                static bool oneTimeWarning = []() { WARNING("[Util::Serialize] Serializing std::chrono::steady_clock gives unpredictable results (the epoch is reset every time the computer reboots)")};
            }
            Serialize(t.time_since_epoch());
        }

        template<class Rep, class Period>
        void Serialize(const std::chrono::duration<Rep, Period>& t, const std::string_view name="") {
            Serialize(t.count(), name);
        }

        template<class T, class Q>
        void Serialize(const std::pair<T, Q>& t, const std::string_view name="") {
            StartClass(2, name);
            Serialize(t.first, "first");
            Serialize(t.second, "second");
            EndClass();
        }
    };

    /**
     * @brief A helper class to make implementing custom deserialization easier
     * Uses CRTP for almost virtual templated functions
     * You need to implement the following methods:
     * ```
     * class CustomDeserializer : public Util::Deserializer<CustomDeserializer> {
     *      void StartClass(const size_t amountMembers, const std::string_view name) override {}
     *      void EndClass() override {}
     *
     *      // The template T will be the type of the container (for example std::vector<uint8_t>)
     *      // You can use reflection/concepts to distinguis different containers
     *      template<class T>
     *      void StartSTLContainer(const std::string_view name) {}
     *      // Template T is the same as for StartSTLContainer
     *      // Should return true if there aren't any more elements left
     *      void IsSTLEnd(const std::string_view name) override {}
     *      // Template T is the same as for StartSTLContainer
     *      template<class T>
     *      void EndSTLContainer() {}
     * 
     *      // Is called with (u)int8_t till (u)int64_t, float, double and long double
     *      // Must set T to the type present in the serialized bytes
     *      template<FundamentalType T>
     *      void GetVariable(T& t, const std::string_view name) {}
     *      void GetVariable(std::string& t, const std::string_view name) {}
     * }
     * ```
     * 
     * @tparam Derived The class that derives and implements all the methods called by Deserializer (see above)
     */
    template<class Derived>
    class Deserializer {
    public:
        
    protected:
        virtual void StartClass(const size_t amountMembers, const std::string_view name) {}
        virtual void EndClass() {}

        template<class T>
        void StartSTLContainer(const std::string_view name) {}
        virtual bool IsSTLEnd() { THROW("[Util::Serialization] IsSTLEnd() must be overriden") }
        template<class T>
        void EndSTLContainer() {}

        template<FundamentalType T>
        void GetVariable(T& t, const std::string_view name) {}
        virtual void GetVariable(std::string& t, const std::string_view name) {}

        /**
         * @brief Counts the publicly accesible members of a class (without the SkipSerialization annotation) and makes sure the class does not contain const members
         * 
         * @tparam T The type
         * @return The amount of member variables 
         */
        template<class T>
        static consteval size_t CountAndCheckMembers() {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            size_t count = 0;
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                static_assert(!std::meta::is_const(std::meta::type_of(memberTypeInfo)), "[Util::Deserializer] Cannot deserialize a class with constant members (please add the Util::SkipSerialization annotation)");
                count++;
            }
            return count;
        }

        /**
         * @brief Will serialize all the members of t accesible to this
         * 
         * @tparam T Object type to serialize
         * @param serializer The serializer specialization to use
         * @param t The object to serialize using the serializer param
         */
        template<class T> requires (!SpecialSerialization<T> && !NotImplementedSerialization<T>)
        void Deserialize(T& t, const std::string_view name="") {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = CountAndCheckMembers<T>();
            if(amountMembers == 0) WARNING("[Util::Deserializer] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members?")
            
            StartClass(amountMembers, name);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if constexpr (HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                else Deserialize(t.[:memberTypeInfo:], std::meta::identifier_of(memberTypeInfo));
            }
            EndClass();
        }

        template<NotImplementedSerialization T>
        void Deserialize(T& t, const std::string_view name) {
            static_assert(false, "[Util::Deserializer] STD library class not yet implemented");
        }

        template<FundamentalType T> requires (!PointerType<T>)
        void Deserialize(T& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template GetVariable<std::remove_cv_t<T>>(t, name);
        }
        void Deserialize(std::string& t, const std::string_view name="") {
            static_cast<Derived*>(this)->GetVariable(t, name);
        }
        template<class T>
        void Deserialize(T*& t, const std::string_view name="") {
            Deserialize(*t, name);
        }
        template<class T>
        void Deserialize(std::shared_ptr<T>& t, const std::string_view name="") {
            t = std::make_shared<T>();
            Deserialize(*t, name);
        }
        template<class T>
        void Deserialize(std::unique_ptr<T>& t, const std::string_view name="") {
            t = std::make_unique<T>();
            Deserialize(*t, name);
        }
        template<class T>
        void Deserialize(std::atomic<T>& t, const std::string_view name="") {
            Deserialize(t.load(), name);
        }

        template<class T> requires IterableSTL<T> && InsertableSTL<T>
        void Deserialize(T& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<T>(name);
            
            while(!IsSTLEnd()) {
                if constexpr (requires { typename T::mapped_type; }) {
                    // Remove the const from the key:
                    std::pair<
                        typename std::remove_cv_t<typename T::key_type>, 
                        typename std::remove_cv_t<typename T::mapped_type>
                    > value;
                    Deserialize(value, "STL-Container-Element");
                    t.insert(t.end(), std::move(value));
                } else {
                    typename std::remove_cv_t<typename T::value_type> value;
                    Deserialize(value, "STL-Container-Element");
                    t.insert(t.end(), std::move(value));
                }
            }

            static_cast<Derived*>(this)->template EndSTLContainer<T>();
        }
        template<class T, class Allocator>
        void Deserialize(std::forward_list<T, Allocator>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::forward_list<T, Allocator>>(name);
            
            while(!IsSTLEnd()) {
                typename std::remove_cv_t<typename T::value_type> value;
                Deserialize(value, "STL-Forward-List-Element");
                t.insert_after(t.end(), std::move(value));
            }

            static_cast<Derived*>(this)->template EndSTLContainer<std::forward_list<T, Allocator>>();
        }
        template<class T> requires (IsTemplatedType<T, ^^std::queue> || IsTemplatedType<T, ^^std::priority_queue> || IsTemplatedType<T, ^^std::stack>)
        void Deserialize(T& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<T>(name);
            while(!IsSTLEnd()) {
                typename std::remove_cv_t<typename T::value_type> value;
                Deserialize(value, "STL-Queue-Element");
                t.push(std::move(value));
            }
            static_cast<Derived*>(this)->template EndSTLContainer<T>();
        }
        template<class T, size_t N>
        void Deserialize(std::array<T, N>& t, const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::array<T, N>>(name);
            for(size_t i = 0; i < N; i++) {
                ASSERT(!IsSTLEnd(), "[Util::Deserializer] Not enough elements serialized for std::array")
                Deserialize(t[i], "STL-Array-Element");
            }
            ASSERT(IsSTLEnd(), "[Util::Deserializer] Too many elements serialized for std::array")
            static_cast<Derived*>(this)->template EndSTLContainer<std::array<T, N>>();
        }
        template<class T, size_t N>
        void Deserialize(T (&t)[N], const std::string_view name="") {
            static_cast<Derived*>(this)->template StartSTLContainer<std::array<T, N>>(name);
            for(size_t i = 0; i < N; i++) {
                ASSERT(!IsSTLEnd(), "[Util::Deserializer] Not enough elements serialized for std::array")
                Deserialize(t[i], "STL-Array-Element");
            }
            ASSERT(IsSTLEnd(), "[Util::Deserializer] Too many elements serialized for std::array")
            static_cast<Derived*>(this)->template EndSTLContainer<std::array<T, N>>();
        }

        template<class Clock, class Duration>
        void Deserialize(std::chrono::time_point<Clock, Duration>& t, const std::string_view name="") {
            if constexpr(IsType<Clock, std::chrono::steady_clock> || IsType<Clock, std::chrono::high_resolution_clock>) {
                static bool oneTimeWarning = []() { WARNING("[Util::Deserializer] Serializing std::chrono::steady_clock and std::chrono::high_resolution_clock give unpredictable results (the epoch is reset every time the computer reboots)"); return true; };
            }
            Duration duration;
            Deserialize(duration);
            t = std::chrono::time_point<Clock, Duration>(std::move(duration));
        }

        template<class Rep, class Period>
        void Deserialize(std::chrono::duration<Rep, Period>& t, const std::string_view name="") {
            Rep count;
            Deserialize(count, name);
            t = std::chrono::duration<Rep, Period>(std::move(count));
        }

        template<class T, class Q>
        void Deserialize(std::pair<T, Q>& t, const std::string_view name="") {
            static_assert(!std::is_const_v<T> && !std::is_const_v<Q>, "[Util::Deserializer] Cannot deserialize a pair with const members");
            StartClass(2, name);
            Deserialize(t.first, "first");
            Deserialize(t.second, "second");
            EndClass();
        }
    };

}
}

#endif