#ifndef ENGINE_UTIL_SERIALIZATION_H
#define ENGINE_UTIL_SERIALIZATION_H

#include "util/BasicLog.h"
#include "util/Reflection.h"
#include "util/TemplateConcepts.h"
#include <memory> // Needed for std::shared_ptr

namespace Engine {
namespace Util {
    
    enum SerializationTypes {
        Class       = 0,
        Pair        = 1,
        Array       = 2,
        Map         = 3,

        UInt8       = 10,
        UInt16      = 11,
        UInt32      = 12,
        UInt64      = 13,
        Int8        = 20,
        Int16       = 21,
        Int32       = 22,
        Int64       = 23,
        Bool        = 24,

        Float       = 30,
        Double      = 31,

        String      = 40
    };
    enum BinarySerializationOutputFlag {
        IncludeTypeInfo     = 1,// Defaults to not include type info
        OutputBigEndian     = 2,// Default is system endianess
        OutputLitleEndian   = 4,// Default is system endianess
        ExcludeVersioning   = 8 // Defaults to including the versioning data
    };

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

    class Serializer;
    class Deserializer;
    class ClassStructureSerializer;
    /**
     * @brief Class can inherit this to make itself serializable
     * Can be used if automatic serialization does not work correctly
     */
    class Serializable {
        /**
         * @brief The function to use while serializing this class, example:
         * ```
         * s->StartClass(1, name);// 1 stands for the amount of members you will serialize
         * s->Serialize(some_member_variable);
         * s->EndClass();
         * 
         * // If you have only one variable, you can just forward the serialization:
         * s->Serialize(some_member_variable, name);
         * ```
         * 
         * @param s The serializer that must be used
         * @param name The member name of the class that holds this type
         */
        virtual void Serialize(Serializer* s, const std::string_view name="") = 0;
        /**
         * @brief The function to use while deserializing this class, example:
         * ```
         * d->StartClass(name);
         * d->Deserialize(some_member_variable);
         * d->EndClass();  
         * 
         * // If you have only one variable, you can just forward the serialization:
         * d->Deserialize(some_member_variable, name);
         * ```
         * 
         * @param d The deserializer that must be used
         * @param name The member name of the class that holds this type
         */
        virtual void Deserialize(Deserializer* d, const std::string_view name="") = 0;

        /**
         * @brief The function to use while serializing this class, example:
         * ```
         * // THESE VALUES YOU MUST OUTPUT if you have multiple variables inside the class:
         * s->Output<uint8_t>(SerializationTypes::Class);
         * s->Output<uint8_t>(amountMembers);
         * s->Output(name);
         * // Then serialize all the member variable types, in the order Serialize() serializes them:
         * s->SerializeInternal<uint64_t>("some_member_variable_name");
         * 
         * // If you have only one variable, you can just forward the serialization:
         * s->SerializeInternal<uint64_t>(name);
         * ```
         * 
         * @param s The serializer that must be used
         * @param name The member name of the class that holds this type
         */
        static void SerializeTypes(ClassStructureSerializer* s, const std::string_view name="") {
            THROW("[Serializable::SerializeTypes] Not implemented")
        }
    };


    template<class T>
    concept SpecialSerialization = 
        Derived<T, Serializable> || FundamentalType<T> || PointerType<T> || ArrayType<T> ||
        StdVectorType<T> || StdMapType<T> || StdStringType<T> || StdTimePoint<T> || StdSharedPtr<T>;



    /**
     * @brief Override to create a custom serializer
     * All these methods must be override so the Deserializer class can correctly identify
     *      the values of variables and for example vector starts/ends
     */
    class Serializer {
    public:
        virtual inline void StartClass(const size_t amountMembers, const std::string_view name="") {}
        virtual inline void EndClass() {}

        virtual inline void StartPair(const std::string_view name="") {}
        virtual inline void EndPair() {}

        virtual inline void StartArray(const size_t amountElements, const std::string_view name="") {}
        virtual inline void EndArray() {}

        // Every key-value pair will be started with StartPair and ended with EndPair
        virtual inline void StartMap(const size_t amountElements, const std::string_view name="") {}
        virtual inline void EndMap() {}

        virtual inline void AddVariable(const int8_t v, const std::string_view name="")  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int16_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int32_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const int64_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const uint8_t v, const std::string_view name="")  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint16_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint32_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const uint64_t v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const bool v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const float v, const std::string_view name="")  { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        virtual inline void AddVariable(const double v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }

        virtual inline void AddVariable(const std::string& v, const std::string_view name="") { THROW("[Util::Deserializer] A AddVariable function is not implemented") }
        
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
        void Serialize(T& t, const std::string_view name="") {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = memberTypes.size();
            if(amountMembers == 0) WARNING("[Util::Serializer] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members? (maybe you forgot to add 'friend class Util::Serializer')")
            
            StartClass(amountMembers, name);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if(HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                Serialize(t.[:memberTypeInfo:], std::meta::identifier_of(memberTypeInfo));
            }
            EndClass();
        }
        /**
         * @name Serialize template overrides for primitives
         */
        ///@{
        template<ClassType T> requires (Derived<T, Serializable>)
        void Serialize(T& t, const std::string_view name="") {
            t.Serialize(this, name);
        }
        template<FundamentalType T> requires (!PointerType<T>)
        void Serialize(T& t, const std::string_view name="") {
            AddVariable(t, name);
        }
        template<StdStringType T>
        void Serialize(T& t, const std::string_view name="") {
            AddVariable(t, name);
        }
        template<class T>
        void Serialize(std::vector<T>& t, const std::string_view name="") {
            StartArray(t.size(), name);
            for(T& c : t) {
                Serialize(c);
            }
            EndArray();        
        }
        template<class T, class Q>
        void Serialize(std::map<T, Q>& t, const std::string_view name="") {
            StartMap(t.size(), name);
            for(auto& [key, value] : t) {
                StartPair();
                Serialize(key);
                Serialize(value);
                EndPair();
            }
            EndMap();
        }
        template<class T, class Q>
        void Serialize(std::chrono::time_point<T, Q>& t, const std::string_view name="") {
            uint64_t count = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::time_point_cast<std::chrono::milliseconds>(t).time_since_epoch()
            ).count();
            AddVariable(count, name);
        }
        template<PointerType T>
        void Serialize(T& t, const std::string_view name="") {
            ASSERT(t != nullptr, "[Util::Serializer] Cannot serialize a nullptr")
            try {
                Serialize(*t, name);
            } catch(...) {
                THROW("[Util::Serializer] Failed to serialize pointer, does it point to a valid adress?")
            }
        }
        template<class T>
        void Serialize(std::shared_ptr<T>& t, const std::string_view name="") {
            Serialize(*t, name);
        }
        template<class T, size_t N>
        void Serialize(T(&t)[N], const std::string_view name="") {
            StartArray(N, name);
            for(size_t i = 0; i < N; i++) {
                Serialize(t[i]);
            }
            EndArray();
        }
        ///@}
        ///@}
    };
    /**
     * @brief Override to create a custom deserializer
     * All these methods must be override so this can correctly identify
     *      the values of variables and for example vector starts/ends serialized by the Serializer class
     * The name given, is the name a variable has inside a class. Name="" if the variable sits inside an array
     */
    class Deserializer {
    public:
        virtual inline void StartClass(const std::string_view name="") {}
        virtual inline void EndClass() {}

        virtual inline void StartPair(const std::string_view name="") {}
        virtual inline void EndPair() {}

        virtual inline void StartArray(const std::string_view name="") {}
        virtual inline bool IsEndArray() { THROW("[Util::Deserializer] A IsEndArray is not implemented") }
        virtual inline void EndArray() {}

        virtual inline void StartMap(const std::string_view name="") {}
        virtual inline bool IsEndMap() { THROW("[Util::Deserializer] A IsEndArray is not implemented") }
        virtual inline void EndMap() {}

        virtual inline void GetVariable(int8_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int16_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int32_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(int64_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(uint8_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint16_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint32_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(uint64_t& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(bool& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(float& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
        virtual inline void GetVariable(double& v, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }

        virtual inline void GetVariable(std::string& s, const std::string_view name="") { THROW("[Util::Deserializer] A GetVariable function is not implemented") }
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
        void Deserialize(T& t, const std::string_view name="") {
            constexpr auto memberTypes = std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
            constexpr size_t amountMembers = memberTypes.size();
            if(amountMembers == 0) WARNING("[Util::Deserializer] Found a class (" + PrettyNameOf<T>() + ") with 0 member variables, are you sure it is intented this class has only private members? (maybe you forgot to add 'friend class Util::Deserializer')")
            
            StartClass(name);
            template for (constexpr std::meta::info memberTypeInfo : memberTypes) {
                if(HasAnnotation<SkipSerializationType>(memberTypeInfo)) continue;
                Deserialize(t.[:memberTypeInfo:], std::meta::identifier_of(memberTypeInfo));
            }
            EndClass();
        }
        /**
         * @name Serialize template overrides 
         */
        ///@{
        template<ClassType T> requires (Derived<T, Serializable>)
        void Deserialize(T& t, const std::string_view name="") {
            t.Deserialize(this, name);
        }
        template<FundamentalType T> requires (!PointerType<T>)
        void Deserialize(T& t, const std::string_view name="") {
            GetVariable(t, name);
        }
        template<StdStringType T>
        void Deserialize(T& t, const std::string_view name="") {
            t = "";
            GetVariable(t, name);
        }
        template<class T>
        void Deserialize(std::vector<T>& t, const std::string_view name="") {
            t.clear();
            StartArray(name);
            while(!IsEndArray()) {
                t.push_back(T());
                Deserialize(t[t.size()-1]);
            }
            EndArray();        
        }
        template<class T, class Q>
        void Deserialize(std::map<T, Q>& t, const std::string_view name="") {
            t.clear();
            StartArray(name);
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
        void Deserialize(std::chrono::time_point<T, Q>& t, const std::string_view name="") {
            long count;
            GetVariable(count, name);
            t = std::chrono::time_point_cast<Q>(
                std::chrono::time_point<T, std::chrono::milliseconds>{
                    std::chrono::milliseconds(count)
                }
            );
        }
        template<PointerType T>
        void Deserialize(T& t, const std::string_view name="") {
            try {
                Deserialize(*t, name);
            } catch(...) {
                t = new std::pointer_traits<T>::type();
                try {
                    Deserialize(*t, name);
                } catch(...) {
                    THROW("[Util::Serializer] Tried to deserialize a pointer, but Serializer failed")
                }
            }
        }
        template<class T>
        void Deserialize(std::shared_ptr<T>& t, const std::string_view name="") {
            t = std::make_shared<T>();
            Deserialize(*t, name);
        }
        template<class T, size_t N>
        void Serialize(T(&t)[N], const std::string_view name="") {
            t.clear();
            StartArray(name);
            for(size_t i = 0; i < N; i++) {
                ASSERT(!IsEndArray(), "[Util::Serialization] Serialized array doesn't contain enough elements")
                Deserialize(t[i]);
            }
            ASSERT(IsEndArray(), "[Util::Serialization] Serialized array containing too many elements")
            EndArray();
        }
        ///@}
        ///@}
    };

}
}

#endif