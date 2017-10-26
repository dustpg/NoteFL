#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>


#include <vector>
#include <algorithm>

enum id : unsigned char {
    id_brackets1 = 0,
    id_brackets2,
    id_plus,
    id_minus,
    id_multip,
    id_division,
    id_firstvalue
};


id get_id(char ch) noexcept {
    switch (ch)
    {
    case '+': return id_plus;
    case '-': return id_minus;
    case '*': return id_multip;
    case '/': return id_division;
    case '(': return id_brackets1;
    case ')': return id_brackets2;
    default: return static_cast<id>(id_firstvalue + (ch - 'a'));
    }
}


inline char get_priority(id i) noexcept {
    return static_cast<char>(i) >> 1;
}

inline bool is_math_symbol(id i) noexcept {
    return i < id_firstvalue;
}

void outputs(const std::vector<id>& o) noexcept {
    for (const auto ch : o) {
        char out = 0;
        switch (ch)
        {
        case id_plus: out = '+'; break;
        case id_minus: out = '-'; break;
        case id_multip: out = '*'; break;
        case id_division: out = '/'; break;
        case id_brackets1: out = '('; break;
        case id_brackets2: out = ')'; break;
        default: out = 'a' + (ch - id_firstvalue); break;
        }
        std::putchar(out);
        std::putchar(' ');
    }
}

void make_tree(const id* data, int len);


int main() {
    //const auto ptr = aligned_alloc;
    // a+b*c+(d*e+f)*g
    // a+((b+c)*d)-e
    std::vector<id> stack, input, output;
    while (const auto ch = std::getchar()) {
        if (ch == '\n') break;
        input.push_back(get_id(ch));
    }


    const auto popup_low_then_push_back = [&stack, &output](id i) {
        const auto this_priority = get_priority(i);
        while (!stack.empty()) {
            const auto last_id = *stack.rbegin();
            if (get_priority(last_id) >= this_priority) {
                output.push_back(last_id);
                stack.pop_back();
            }
            else break;
        }
        stack.push_back(i);
    };

    const auto popup_brackets = [&stack, &output] {
        while (true) {
            const auto this_id = *stack.rbegin();
            stack.pop_back();
            if (this_id == id_brackets1) break;
            else output.push_back(this_id);
        };
    };

    // STEP1
    for (const auto i : input) {
        switch (i)
        {
        case id_brackets1: stack.push_back(i); break;
        case id_brackets2: popup_brackets(); break;
        case id_plus: //[[fallthrough]]
        case id_minus: //[[fallthrough]]
        case id_multip: //[[fallthrough]]
        case id_division: popup_low_then_push_back(i); break;
        default: output.push_back(i);
        }
    }
    // STEP2
    while (!stack.empty()) {
        output.push_back(*stack.rbegin());
        stack.pop_back();
    }

    outputs(output);
    std::putchar('\n');

    make_tree(output.data(), output.size());

    std::getchar();
    return 0;
}



class ExpressionTree {
public:
    struct Node {
        const Node* parent;
        const Node* left;
        const Node* right;
        id      data;
    };
public:
    ExpressionTree(const id* data, int len) noexcept;
    ~ExpressionTree() noexcept;
    ExpressionTree(const ExpressionTree&) noexcept = delete;
    const Node* Root() const noexcept { return m_pRoot; }
    void Print() const noexcept;
private:
    void*           m_pAllData = nullptr;
    Node*           m_pRoot = nullptr;
    const    int    m_len;
};

ExpressionTree::ExpressionTree(const id * data, int len) noexcept :
m_pAllData(std::malloc((sizeof(Node)+sizeof(void*))*len)), m_len(len) {
    if (!m_pAllData) return;
    const auto ptr = reinterpret_cast<Node*>(m_pAllData);
    const auto stack = reinterpret_cast<Node**>(ptr + len);

    auto stack_top = stack;
    auto malloced = ptr;

    const auto push_to_stack = [=, &stack_top](Node* node) noexcept {
        *stack_top = node;
        ++stack_top;
        assert(stack_top <= stack + len);
    };
    const auto popup_two = [=, &stack_top]() noexcept {
        stack_top -= 2;
        assert(stack_top >= stack);
    };

    for (int i = 0; i != len; ++i) {
        const auto this_id = data[i];
        assert(malloced < reinterpret_cast<Node*>(stack));

        malloced->data = this_id;
        malloced->left = nullptr;
        malloced->right = nullptr;
        malloced->parent = nullptr;

        if (!is_math_symbol(this_id)) {
            push_to_stack(malloced);
        }
        else {
            assert((stack_top - 2) >= stack);
            const auto left = stack_top[-2];
            left->parent = malloced;
            malloced->left = left;
            const auto right = stack_top[-1];
            right->parent = malloced;
            malloced->right = right;
            popup_two();
            push_to_stack(malloced);
            m_pRoot = malloced;
        }
        ++malloced;
    }
}

ExpressionTree::~ExpressionTree() noexcept {
    std::free(m_pAllData);
}


void make_tree(const id* data, int len) {
    ExpressionTree tree{ data, len };
    // 有BUG, 会重叠, 不过估计很麻烦就不解决了
    tree.Print();
    /*
            a
           / \
          b   c
    */
}


struct TMP { const ExpressionTree::Node* node; int x, y; };
void MakePrint(TMP*& ptr, const ExpressionTree::Node* node, int x, int y) {
    ptr->node = node;
    ptr->x = x;
    ptr->y = y;
    if (node->left) {
        ++ptr;
        MakePrint(ptr, node->left, x - 2, y + 2);
    }
    if (node->right) {
        ++ptr;
        MakePrint(ptr, node->right, x + 2, y + 2);
    }
}


void ExpressionTree::Print() const noexcept {
    try {
        const auto len = m_len;
        const auto root = this->Root();
        std::vector<TMP> buffer;
        buffer.resize(len);
        auto first = &*buffer.begin();
        MakePrint(first, root, 0, 0);
        const auto xx = std::minmax_element(buffer.begin(), buffer.end(), 
            [](const TMP&a, const TMP& b) noexcept { return a.x < b.x; });
        const auto yy = std::max_element(buffer.begin(), buffer.end(), 
            [](const TMP&a, const TMP& b) noexcept { return a.y < b.y; });

        const auto width = xx.second->x - xx.first->x + 2;
        const auto height = yy->y + 2;

        std::vector<char> out;
        out.assign(width*height, ' ');

        for (int i = 0; i != height; ++i)
            out[(i + 1)*width - 1] = '\n';

        const auto ox = -xx.first->x;
        for (int i = 0; i != len; ++i) {
            const auto x = buffer[i].x + ox;
            const auto y = buffer[i].y;
            const auto this_id = buffer[i].node->data;
            const auto c = [](const id ch) -> char {
                switch (ch)
                {
                case id_plus: return '+'; 
                case id_minus: return '-'; 
                case id_multip: return'*'; 
                case id_division: return '/'; 
                case id_brackets1:return '('; 
                case id_brackets2: return ')'; 
                default: return 'a' + (ch - id_firstvalue);
                }
            }(this_id);

            out[x + y * width] = c;
            if (is_math_symbol(this_id)) {
                out[(x - 1) + (y + 1) * width] = '/';
                out[(x + 1) + (y + 1) * width] = '\\';
            }
        }
        *out.rbegin() = '\0';
        std::putchar('\n');
        std::puts(out.data());

    }
    catch (...) {
        std::printf("ERROR!\n");
    }
}