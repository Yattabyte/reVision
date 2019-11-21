#pragma once
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <algorithm>
#include <array>
#include <any>
#include <optional>
#include <map>
#include <string>
#include <tuple>
#include <vector>


/***/
class Serializer {
public:
	/***/
	template <typename FirstMember, typename ...RemainingMembers>
	static std::vector<char> Serialize_Members(const FirstMember& first, const RemainingMembers& ...rest) {
		// Retrieve the first member's serialized data
		const auto& [MemberName, MemberContainer] = first;
		auto memberData = Serialize_Data(MemberName, MemberContainer);

		// For each remaining member of the parameter pack, recursively call this function, appending the results
		if constexpr (sizeof...(rest) > 0) {
			const auto nextMemberData = Serialize_Members(rest...);
			memberData.insert(memberData.end(), nextMemberData.begin(), nextMemberData.end());
		}

		return memberData;
	}
	/***/
	template <class T>
	static std::vector<char> Serialize_Data(const std::string& name, const T& data) {
		// For convenience sake, wrap our output data into a memory-copyable struct
		struct Memory_Structure {
			const int struct_size = (int)sizeof(Memory_Structure);
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			const int payload_size = (int)sizeof(T);
			char payload_data[sizeof(T)];

			/** Fill this memory structure. */
			Memory_Structure(const std::string& name, const T& data) {
				// Copy-in the variable name, clamped to a max of MAX_NAME_CHARS)
				std::memcpy(&payload_name[0], name.c_str(), std::min(name.size(), (size_t)MAX_NAME_CHARS));
				// Copy-in the variable data
				std::memcpy(&payload_data[0], &data, payload_size);
			}
		} const outputData(name, data);

		// Allocate a buffer for our data, copy into it, and write pass it back
		std::vector<char> dataBuffer(sizeof(Memory_Structure));
		std::memcpy(&dataBuffer[0], &outputData, sizeof(Memory_Structure));
		return dataBuffer;
	}
	template <>
	static std::vector<char> Serialize_Data(const std::string& name, const std::string& data) {
		// For convenience sake, wrap our output data into a memory-copyable struct
		struct Memory_Structure {
			int struct_size = (int)sizeof(Memory_Structure);
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			int payload_size;

			/** Fill this memory structure. */
			Memory_Structure(const std::string& name, const std::string& data) {
				// Update size variables with data size
				payload_size = (int)(sizeof(char) * data.size());
				struct_size += payload_size;
				// Copy-in the variable name, clamped to a max of MAX_NAME_CHARS)
				std::memcpy(&payload_name[0], name.c_str(), std::min(name.size(), (size_t)MAX_NAME_CHARS));
			}
		} const outputData(name, data);

		// Allocate a buffer for our data, copy into it, and write pass it back
		std::vector<char> dataBuffer(outputData.struct_size);
		std::memcpy(&dataBuffer[0], &outputData, sizeof(Memory_Structure));
		// Copy-in the variable data
		std::memcpy(&dataBuffer[sizeof(Memory_Structure)], &data[0], sizeof(char) * data.size());
		return dataBuffer;
	}
	/***/
	template <typename ...Members>
	static void Deserialize_Members(const std::vector<char>& memberData, Members& ...members) {
		if (memberData.size()) {
			std::map<std::string, std::vector<char>> memberMap;

			// Populate map with member info from buffer data
			constexpr static auto Fill_Member_Map = [](auto& memberMap, const auto& memberData) {
				size_t index(0ULL), prevIndex(0ULL);

				while (index < memberData.size()) {
					// Get the memory structure size
					const int& memory_struct_size = *reinterpret_cast<int*>(const_cast<char*>(&memberData[index]));
					index += sizeof(int);

					// Get the memory payload's name
					char memory_payload_name[MAX_NAME_CHARS]{ '\0' };
					std::memcpy(&memory_payload_name[0], &memberData[index], (size_t)MAX_NAME_CHARS);
					const std::string memory_payload_string(memory_payload_name, MAX_NAME_CHARS);
					index += sizeof(char) * (size_t)MAX_NAME_CHARS;

					// Get the memory payload's size
					const int& memory_payload_size = *reinterpret_cast<int*>(const_cast<char*>(&memberData[index]));
					index += sizeof(int);

					// Get the memory payload's data
					char* memory_payload_data = const_cast<char*>(&memberData[index]);
					index += sizeof(char) * (size_t)memory_payload_size;

					// Insert clamped string with memory data into map
					memberMap.insert({ memory_payload_string.c_str(), std::vector<char>(memberData.begin() + prevIndex, memberData.begin() + index) });
					prevIndex = index;
				}
			};
			Fill_Member_Map(memberMap, memberData);

			// Iterate through the expected member names
			Search_And_Assign_Members(memberMap, members...);
		}
	}
	/***/
	template <class T>
	static std::optional<std::pair<std::string, T>> Deserialize_Data(const std::vector<char>& dataBuffer) {
		// The expected structure of the input data
		struct Memory_Structure {
			const int struct_size = (int)sizeof(Memory_Structure);
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			const int payload_size = (int)sizeof(T);
			char payload_data[sizeof(T)];
		};

		// Ensure the data buffer is of the expected size
		if (dataBuffer.size() == (int)sizeof(Memory_Structure)) {
			// Cast the memory back into the structure
			const auto& inputData = *reinterpret_cast<Memory_Structure*>(const_cast<char*>(&dataBuffer[0]));
			// Ensure internal memory size matches
			if (dataBuffer.size() == inputData.struct_size) {
				const std::string name(inputData.payload_name, MAX_NAME_CHARS);
				const T& data = *reinterpret_cast<T*>(const_cast<char*>(&inputData.payload_data[0]));
				return { { name,data } };
			}
		}
		return {};
	}
	/***/
	template <>
	static std::optional<std::pair<std::string, std::string>> Deserialize_Data(const std::vector<char>& dataBuffer) {
		// The expected structure of the input data
		struct Memory_Structure {
			int struct_size = (int)sizeof(Memory_Structure);
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			int payload_size;
		};

		// Ensure the data buffer is of the expected size
		if (dataBuffer.size() >= (int)sizeof(Memory_Structure)) {
			// Cast the memory back into the structure
			const auto& inputData = *reinterpret_cast<Memory_Structure*>(const_cast<char*>(&dataBuffer[0]));
			// Ensure internal memory size matches
			if (dataBuffer.size() == inputData.struct_size) {
				const std::string name(inputData.payload_name, MAX_NAME_CHARS);
				std::string payload_string(&dataBuffer[sizeof(Memory_Structure)], inputData.payload_size);
				return { { name, payload_string } };
			}
		}
		return {};
	}


private:
	// Private Methods
	/***/
	template <typename FirstMember, typename ...RemainingMembers>
	static void Search_And_Assign_Members(const std::map<std::string, std::vector<char>>& memberMap, FirstMember& first, RemainingMembers& ...rest) {
		// Try to find the first member of this parameter pack in the member map
		auto& [MemberName, MemberPointer] = first;
		const auto memberNameSpot = memberMap.find(std::string(MemberName));
		if (memberNameSpot != memberMap.end()) {
			// Found member, de-serialize data
			if (const auto qwe = Deserialize_Data<std::remove_pointer<decltype(MemberPointer)>::type>(memberMap.at(MemberName)))
				*MemberPointer = qwe->second;
		}

		if constexpr (sizeof...(rest) > 0)
			Search_And_Assign_Members(memberMap, rest...);
	}


	// Private Variables
	constexpr static int MAX_NAME_CHARS = 32;
};

#endif // SERIALIZER_H