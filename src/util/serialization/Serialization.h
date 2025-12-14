#ifndef ENGINE_UTIL_SERIALIZATION_H
#define ENGINE_UTIL_SERIALIZATION_H

#include "core/PCH.h"

namespace Engine {
namespace Util {
    
    class Serializer;
    class Deserializer;
    /**
     * @brief Class can inherit this to make itself serializable
     * Can be used if automatic serialization does not work correctly
     */
    class Serializable {
        /**
         * @brief The function to use while serializing this class, example:
         * ```
         * s->StartClass(1);// 1 stands for the amount of members you will serialize
         * Serialize(*s, 1);
         * s->EndClass();
         * ```
         * 
         * @param s The serializer that must be used
         */
        virtual void Serialize(Serializer* s) = 0;
        /**
         * @brief The function to use while deserializing this class, example:
         * ```
         * d->StartClass();
         * Deserialize(*d, 1);
         * d->EndClass();
         * ```
         * 
         * @param d The deserializer that must be used
         */
        virtual void Deserialize(Deserializer* d) = 0;
    };


    template<class T>
    concept SpecialSerialization = 
        Derived<T, Serializable> || FundamentalType<T> || PointerType<T> || 
        StdVectorType<T> || StdMapType<T> || StdStringType<T> || StdTimePoint<T>;



    /**
     * @brief Override to create a custom serializer
     * All these methods must be override so the Deserializer class can correctly identify
     *      the values of variables and for example vector starts/ends
     */
    class Serializer {
    public:
        virtual inline void StartClass(const size_t amountMembers) {}
        virtual inline void EndClass() {}

        virtual inline void StartPair() {}
        virtual inline void EndPair() {}

        virtual inline void StartArray(const size_t amountElements) {}
        virtual inline void EndArray() {}

        virtual inline void AddVariable(const int8_t v)  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int16_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int32_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int64_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const uint8_t v)  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint16_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint32_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint64_t v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const float v)  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const double v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const std::string& v) { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        
        /**
        * @name Serialization
        */
        ///@{

        /**
         * @brief Will serialize the param t
         * Has a lot of template overrides for primitive types.
         * If you want to use these methods, the class must be either fully public, or implement:
         *      tuple_size<T>, tuple_element<I, T> and get<I> (so structured bindings work)
         * This requirement must also hold for all it's members.
         * 
         * @tparam T Object type to serialize
         * @param serializer The serializer specialization to use
         * @param t The object to serialize using the serializer param
         */
        template<class T> requires (!SpecialSerialization<T>)
        void Serialize(T& t) {
            auto&[...pack] = t;
            StartClass(sizeof...(pack));
            SerializeComponents(pack...);
            EndClass();
        }
        template<class T, class ...Ts>
        inline void SerializeComponents(T& first, Ts&... pack) {
            Serialize(first);
            SerializeComponents(pack...);
        }
        template<class T>
        inline void SerializeComponents(T& first) {
            // End of recursion
            Serialize(first);
        }
        /**
         * @name Serialize template overrides for primitives
         */
        ///@{
        template<ClassType T> requires (Derived<T, Serializable>)
        void Serialize(T& t) {
            t.Serialize(this);
        }
        template<FundamentalType T>
        void Serialize(T& t) {
            AddVariable(t);
        }
        template<StdStringType T>
        void Serialize(T& t) {
            AddVariable(t);
        }
        template<class T>
        void Serialize(std::vector<T>& t) {
            StartArray(t.size());
            for(T& c : t) {
                Serialize(c);
            }
            EndArray();        
        }
        template<class T, class Q>
        void Serialize(std::map<T, Q>& t) {
            StartArray(t.size());
            for(auto& [key, value] : t) {
                StartPair();
                Serialize(key);
                Serialize(value);
                EndPair();
            }
            EndArray();        
        }
        template<class T, class Q>
        void Serialize(std::chrono::time_point<T, Q>& t) {
            AddVariable(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::time_point_cast<std::chrono::milliseconds>(t).time_since_epoch()
            ).count());
        }
        template<PointerType T>
        void Serialize(T& t) {
            Serialize(*t);
        }
        ///@}
        ///@}
    };
    /**
     * @brief Override to create a custom deserializer
     * All these methods must be override so this can correctly identify
     *      the values of variables and for example vector starts/ends serialized by the Serializer class
     */
    class Deserializer {
    public:
        virtual inline void StartClass() {}
        virtual inline void EndClass() {}

        virtual inline void StartPair() {}
        virtual inline void EndPair() {}

        virtual inline void StartArray() {}
        virtual inline bool IsEndArray() { THROW("[Util::Deserializer] A IsEndArray is not implemented") }
        virtual inline void EndArray() {}

        virtual inline void GetVariable(int8_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int16_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int32_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int64_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(uint8_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint16_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint32_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint64_t& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(float& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(double& v) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(std::string& s) { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        /**
         * @name Deserialization
         */
        ///@{

        /**
         * @brief Will deserialize the param t
         * Has template overrides for primitive types.
         * 
         * @tparam T Object type to deserialize
         * @param deserialize The deserializer specialization to use
         * @param t The object to deserialize using the deserializer param
         */
        template<class T> requires (!SpecialSerialization<T>)
        void Deserialize(T& t) {
            StartClass();
            auto&[...pack] = t;
            DeserializeComponents(pack...);
            EndClass();
        }
        template<class T, class ...Ts>
        inline void DeserializeComponents(T& first, Ts&... pack) {
            Deserialize(first);
            DeserializeComponents(pack...);
        }
        template<class T>
        inline void DeserializeComponents(T& first) {
            // End of recursion
            Deserialize(first);
        }
        /**
         * @name Serialize template overrides 
         */
        ///@{
        template<ClassType T> requires (Derived<T, Serializable>)
        void Deserialize(T& t) {
            t.Deserialize(this);
        }
        template<FundamentalType T>
        void Deserialize(T& t) {
            GetVariable(t);
        }
        template<StdStringType T>
        void Deserialize(T& t) {
            GetVariable(t);
        }
        template<class T>
        void Deserialize(std::vector<T>& t) {
            t.clear();
            StartArray();
            while(!IsEndArray()) {
                t.push_back();
                Deserialize(t[t.size()-1]);
            }
            EndArray();        
        }
        template<class T, class Q>
        void Deserialize(std::map<T, Q>& t) {
            t.clear();
            StartArray();
            while(!IsEndArray()) {
                StartPair();
                T key{};
                Deserialize(key);
                Deserialize(t[key]);
                EndPair();
            }
            EndArray();        
        }
        template<class T, class Q>
        void Deserialize(std::chrono::time_point<T, Q>& t) {
            long count;
            GetVariable(count);
            t = std::chrono::time_point_cast<Q>(
                std::chrono::time_point<T, std::chrono::milliseconds>{
                    std::chrono::milliseconds(count)
                }
            );
        }
        template<PointerType T>
        void Deserialize(T& t) {
            Deserialize(*t);
        }
        ///@}
        ///@}
    };

}
}

#endif