#ifndef BINSORT_H
#define BINSORT_H
#include <array>
#include <vector>

namespace godot {
class IBinSortable {
public:
	int bin;
};

class BinSort {
public:
	static int get_bin_number(int i, int j, int n) {
		return (i % 2 == 0) ? (i * n) + j : (i + 1) * n - j - 1;
	}

	template<typename T>
	static std::vector<T> sort(std::vector<T> input, int lastIndex, int binCount) {
		size_t n = input.size();
		std::vector<int> count(binCount, 0);
		std::vector<T> output(n);

		if (binCount <= 1) return input;
		if (lastIndex > (int)n) lastIndex = (int)n;

		for (int i = 0; i < lastIndex; i++) {
			int j = input[i].bin;
			count[j] += 1;
		}
		for (int i = 1; i < binCount; i++) {
			count[i] += count[i - 1];
		}
		for (int i = lastIndex - 1; i >= 0; i--) {
			int j = input[i].bin;
			count[j] -= 1;
			output[count[j]] = input[i];
		}
		for (int i = lastIndex; i < (int)n; i++) {
			output[i] = input[i];
		}
		return output;
	}

};

}
#endif