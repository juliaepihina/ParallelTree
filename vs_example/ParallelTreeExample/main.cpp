#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include <string>

#include "ParallelTree.hpp"

using namespace std;

std::string first, second, result;
int letters_count = 0;

//замена определенной буквы на определенную цифру
void replace(std::string& s, char c, int n) {
	for (auto& i : s)
		if (i == c)
			i = n + '0';
}

//добавляет единицу к строке, максимум до 999...9
std::string add_one(std::string x)
{
	x[x.length() - 1] += 1;
	for (int i = x.length() - 1; i != 0; i--)
	{
		if (x[i] == '9' + 1)
		{
			x[i] = '0';
			if (i == 0)
				return x;
			x[i - 1]++;
		}
	}

	return x;
}

//подставляет цифры вместо букв в строках и проверяет, равна ли сумма
bool paste_check(std::string nums) {
	std::map<char, int> letters;
	int j = 0;

	for (auto i : first + second + result)
		letters[i] = 0;//nums[j++] - '0';
	for (auto& i : letters)
		letters[i.first] = nums[j++] - '0';

	std::string first_r = first, second_r = second, result_r = result;

	for (auto i : letters) {
		replace(first_r, i.first, i.second);
		replace(second_r, i.first, i.second);
		replace(result_r, i.first, i.second);
	}

	return std::stoi(first_r) + std::stoi(second_r) == std::stoi(result_r);
}

// Рекорд. Должен наследоваться от класса Record и реализовать методы
// betterThan и clone.
class ExampleRecord : public Record {
public:
	ExampleRecord() : x("") {
	}

	// Минимальное найденное значение узла
	std::string x;

	/*
	 * Должна возвращать true, если данный рекорд лучше (меньше в задачах
	 * минимизации и больше в задачах максимизации), чем other
	 */
	bool betterThan(const Record& other) const override {
		const ExampleRecord& otherCast =
			static_cast<const ExampleRecord&>(other);
		// Поскольку у нас задача максимизации, то используем оператор "больше".
		return paste_check(x);
	}

	// Должен возвращать копию данного рекорда.
	virtual std::unique_ptr<Record> clone() const override {
		// Здесь просто используем конструктор копий
		return std::make_unique<ExampleRecord>(*this);
	}
};

// Узел дерева вариантов. Должен наследоваться от класса Node и реализовать
// методы process и hasHigherPriority.
class ExampleNode : public Node {
public:
	ExampleNode(std::string value) : x(value) {
	}

	// Значение в данном узле дерева.
	std::string x;

	/*
	 * Функция, которая обрабатывает текущий узел и возвращает вектор
	 * потомков этого узла (или пустой вектор, если потомков нет).
	 *
	 * Она не должна менять глобальных переменных, т.к. она будет исполняться
	 * в нескольких потоках. Рекорд менять можно (при этом синхронизация не
	 * требуется).
	 */
	virtual std::vector<std::unique_ptr<Node>>
		process(Record& record) override {
		ExampleRecord& recordCast = static_cast<ExampleRecord&>(record);

		if (paste_check(x)) {
			recordCast.x = x;
		}

		std::string max;

		for (int i = 0; i < letters_count; i++) {
			max += '9';
		}

		// Потомки
		std::vector<std::unique_ptr<Node>> childNodes;

		if (x != max)
		{
			std::cout << x << std::endl;
			childNodes.emplace_back(new ExampleNode(add_one(x)));

			//std::string rvrs = add_one(x);
			//std::reverse(rvrs.begin(), rvrs.end());
			//childNodes.emplace_back(new ExampleNode(rvrs));
			//childNodes.emplace_back(new ExampleNode(add_one_last(x)));
			// childNodes.emplace_back(new ExampleNode(x - 1));
		}
		return childNodes;
	}

	/*
	 * Возвращает true, если приоритет данного задания больше, чем other.
	 * Задания с большим приоритетом будут обрабатываться раньше.
	 */
	virtual bool hasHigherPriority(const Node& other) const override {
		const ExampleNode& otherCast = static_cast<const ExampleNode&>(other);
		// Если у данного узда значение f больше, то считаем что у него больше
		// приоритет.
		return std::stoi(x) < std::stoi(otherCast.x);
	}
};

int main() {
	std::ifstream f("test.txt");
	f >> first;
	f >> second;
	f >> result;
	f.close();
	std::map<char, int> letters;
	int j = 0;

	for (auto i : first + second + result)
		letters[i] = 0;

	std::string startstr = "";

	for (auto i : letters) {
		letters_count++;
		startstr += "0";
	}

	ExampleRecord initialRecord;
	// Корень дерева вариантов.
	unique_ptr<ExampleNode> root = make_unique<ExampleNode>(startstr);
	auto startTime = chrono::high_resolution_clock::now();
	// Параллельно находим решение
	unique_ptr<Record> bestSolution =
		parallelTree(move(root), initialRecord, 4);
	const ExampleRecord* bestSolutionCast =
		reinterpret_cast<const ExampleRecord*>(bestSolution.get());
	auto finishTime = chrono::high_resolution_clock::now();
	auto duration =
		chrono::duration_cast<chrono::microseconds>(finishTime - startTime);


	auto x = bestSolutionCast->x;
	int k = 0;

	std::ofstream out("output.txt");
	for (const auto& i : letters) {
		std::cout << i.first << " = " << x[k] << std::endl;
		out << i.first << " = " << x[k++] << std::endl;
	}

	
	cout << "finished in " << duration.count() << " microseconds" << endl;
	out << "finished in " << duration.count() << " microseconds" << endl;
	out.close();

	return 0;
}