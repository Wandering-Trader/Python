#include "int2048.h"
namespace sjtu
{
    int string_to_int(const std::string &s)
    {
        int res = 0;
        for (auto c : s)
        {
            res *= 10;
            res += c - '0';
        }
        return res;
    }
    int2048::int2048()
    {
        len = 1;
        neg = false;
        num.clear();
        num.push_back(0);
    }
    // give a number, default 0
    int2048::int2048(long long x)
    {
        len = 0;
        neg = false;
        unsigned long long tmp;
        if (x < 0)
        {
            neg = true;
            tmp = -(x + 1) + 1;
        }
        else
            tmp = x;
        num.clear();
        if (tmp == 0) // special judge whether x = 0
        {
            len = 1;
            num.push_back(0);
            return;
        }
        while (tmp)
        {
            num.push_back(tmp % BASE);
            tmp /= BASE;
            ++len;
        }
    }
    // erase the highest 0s
    inline void int2048::erase0()
    {
        // in case the number itself is 0.
        while (len > 1 && num.back() == 0)
        {
            --len;
            num.pop_back();
        }
        if (neg && len == 1 && num.back() == 0)
            neg = false;
    }
    int2048 &int2048::add(const int2048 &B)
    {
        bool Neg = neg;
        if (neg == B.neg)
        {
            *this = add_abs(*this, B);
            neg = Neg;
            carry();
            return *this;
        }
        int cmp = comp_abs(*this, B);
        *this = minus_abs(*this, B);
        // |A| > |B|
        if (cmp == 1)
            neg = Neg;
        else if (cmp == -1)
            neg = B.neg;
        else
            neg = false;
        carry();
        return *this;
    }
    int2048 &int2048::minus(const int2048 &B)
    {
        int2048 C = B;
        C.neg = !C.neg;
        add(C);
        return *this;
    }
    int2048 int2048::operator+() const { return *this; }
    int2048 int2048::operator-() const
    {
        int2048 A = *this;
        if (A.len == 1 && A.num.back() == 0)
            return A;
        A.neg = !A.neg;
        return A;
    }
    int2048 &int2048::operator=(const int2048 &B)
    {
        len = B.len;
        num = B.num;
        neg = B.neg;
        return *this;
        // TODO: insert return statement here
    }
    int2048 &int2048::operator+=(const int2048 &B)
    {
        add(B);
        return *this;
        // TODO: insert return statement here
    }
    int2048 &int2048::operator-=(const int2048 &B)
    {
        minus(B);
        return *this;
        // TODO: insert return statement here
    }
    int2048 &int2048::operator*=(const int2048 &B)
    {
        *this = *this * B;
        return *this;
        // TODO: insert return statement here
    }
    int2048 &int2048::operator/=(const int2048 &B)
    {
        *this = *this / B;
        return *this;
        // TODO: insert return statement here
    }
    int2048 &int2048::operator%=(const int2048 &B)
    {
        return *this = *this % B;
    }
    int2048::operator double() const
    {
        return turnDouble(*this);
    }
    void int2048::read(const std::string &s) { *this = int2048(s); }
    // actually this function handles both carries and borrows
    inline void int2048::carry(bool flag)
    {
        num.push_back(0);
        for (int i = 0; i < len; ++i)
        {
            if (num[i] >= BASE)
            {
                num[i + 1] += num[i] / BASE;
                num[i] %= BASE;
            }
            if (num[i] < 0)
            {
                LL x = (-num[i] - 1) / BASE + 1;
                num[i] += BASE * x;
                assert(num[i] >= 0 && num[i] < BASE);
                num[i + 1] -= x;
            }
        }
        if (flag)
        {
            len = num.size();
            erase0();
        }
    }
    void int2048::print(std::ostream &os)
    {
        erase0();
        if (neg)
            os << "-";
        os << num.back();
        int sz = num.size();
        for (int i = sz - 2; ~i; --i)
            os << std::setw(WIDTH) << std::setfill('0') << num[i];
    }
    int2048::int2048(const std::string &str)
    {
        int st = 0;
        if (str[0] == '-')
        {
            neg = true;
            st = 1;
        }
        else
            neg = false;
        len = 0;
        num.clear();
        int lenS = str.length();
        while (lenS > 0)
        {
            int front = std::max(st, lenS - WIDTH);
            int LEN = std::min(WIDTH, lenS - front);
            if (LEN == 0)
                break;
            ++len;
            num.push_back(string_to_int(str.substr(front, LEN)));
            lenS -= WIDTH;
        }
        erase0();
    }
    int2048::int2048(const int2048 &B)
    {
        neg = B.neg;
        len = B.len;
        num = B.num;
    }
    // 1: |A| > |B|  0: |A| = |B|  -1: |A| < |B|
    int comp_abs(const int2048 &A, const int2048 &B)
    {
        if (A.len > B.len)
            return 1;
        else if (A.len < B.len)
            return -1;
        for (int i = A.len - 1; ~i; --i)
            if (A.num[i] > B.num[i])
                return 1;
            else if (A.num[i] < B.num[i])
                return -1;
        return 0;
    }
    // calculate |A|+|B|
    int2048 add_abs(const int2048 &A, const int2048 &B)
    {
        int2048 ans;
        ans.len = std::max(A.len, B.len);
        ans.num.resize(ans.len);
        for (int i = 0; i < ans.len; ++i)
        {
            int a = i < A.len ? A.num[i] : 0;
            int b = i < B.len ? B.num[i] : 0;
            ans.num[i] = a + b;
        }
        ans.carry();
        return ans;
    }
    // calculate ||A|-|B||
    int2048 minus_abs(const int2048 &A, const int2048 &B)
    {
        int cmp = comp_abs(A, B);
        if (cmp == 0)
            return int2048(0);
        int2048 ans;
        ans.len = std::max(A.len, B.len);
        ans.num.resize(ans.len);
        for (int i = 0; i < ans.len; ++i)
        {
            int a = i < A.len ? A.num[i] : 0;
            int b = i < B.len ? B.num[i] : 0;
            ans.num[i] = cmp == 1 ? a - b : b - a;
        }
        ans.carry();
        return ans;
    }

    int2048 add(int2048 A, const int2048 &B)
    {
        A.add(B);
        return A;
    }

    int2048 minus(int2048 A, const int2048 &B)
    {
        A.minus(B);
        return A;
    }

    int2048 operator+(int2048 A, const int2048 &B) { return add(A, B); }

    int2048 operator-(int2048 A, const int2048 &B) { return minus(A, B); }

    inline int rev(int x, int p)
    {
        for (int k = 0; k < p - 1 - k; ++k)
        {
            if (((x >> k) & 1) ^ ((x >> (p - 1 - k)) & 1))
            {
                x ^= (1 << k);
                x ^= (1 << (p - 1 - k));
            }
        }
        return x;
    }
    void DFT(std::vector<std::complex<double>> &A, int p, int mode)
    {
        int n = (1 << p);
        const double PI = acos(-1);
        double theta = 2 * PI / n;
        std::vector<std::complex<double>> omega;
        omega.reserve(n);
        for (int i = 0; i < n; ++i)
            omega.push_back({cos(theta * i), sin(theta * i)});
        for (int i = 0; i < n; ++i)
        {
            int j = rev(i, p);
            if (i < j)
                swap(A[i], A[j]);
        }
        int len_w = n;
        for (int k = 0; k < p; ++k)
        {
            int Len = (1 << k);
            len_w >>= 1;
            int id_w = 0;
            for (int i = 0; i < n; ++i)
            {
                if ((i >> k) & 1)
                {
                    i += Len;
                    id_w = 0;
                    if (i >= n)
                        continue;
                }
                std::complex<double> x = A[i];
                std::complex<double> y = A[i + Len] * omega[id_w];
                A[i] = x + y;
                A[i + Len] = x - y;
                id_w += len_w;
            }
        }
        if (mode == -1)
        {
            for (int i = 1; i < n - i; ++i)
                swap(A[i], A[n - i]);
            for (int i = 0; i < n; ++i)
                A[i] /= n;
        }
    }

    int2048 operator*(int2048 A, const int2048 &B)
    {
        int2048 ans;
        int fft_len = A.len + B.len - 1;
        ans.len = fft_len;
        int p = 0;
        while ((1 << p) <= fft_len)
            ++p;
        fft_len = (1 << p);
        std::vector<std::complex<double>> c, c1, c2;
        c.clear();
        c1.clear();
        c2.clear();
        c.reserve(fft_len);
        c1.reserve(fft_len);
        c2.reserve(fft_len);
        for (int i = 0; i < fft_len; ++i)
        {
            double x = (i < A.len ? A.num[i] : 0);
            double y = (i < B.len ? B.num[i] : 0);
            c1.push_back({x, y});
        }
        DFT(c1, p, 1);
        for (int i = 0; i < fft_len; ++i)
            c2.push_back(conj(c1[(fft_len - i) & (fft_len - 1)]));
        for (int i = 0; i < fft_len; ++i)
        {
            std::complex<double> A, B;
            A = 0.5 * (c1[i] + c2[i]);
            B = (std::complex<double>){0, -0.5} * (c1[i] - c2[i]);
            c1[i] = A;
            c2[i] = B;
        }
        for (int i = 0; i < fft_len; ++i)
            c.push_back(c1[i] * c2[i]);
        DFT(c, p, -1);
        ans.num.resize(ans.len);
        for (int i = 0; i < ans.len; ++i)
            ans.num[i] = (LL)(c[i].real() + 0.5);
        ans.neg = A.neg ^ B.neg;
        ans.carry();
        return ans;
    }
    // compare the highest bits of A and B (with the same width)
    int comp_width(const int2048 &A, const int2048 &B)
    {
        if (A.len < B.len)
            return -1;
        for (int i = 1; i <= B.len; ++i)
        {
            int posA = A.len - i, posB = B.len - i;
            if (A.num[posA] < B.num[posB])
                return -1;
            else if (A.num[posA] > B.num[posB])
                return 1;
        }
        return 0;
    }
    double turnDouble(const int2048 &B)
    {
        double ans = 0;
        for (int i = 0; i < B.len; ++i)
            ans = ans * int2048::BASE + B.num[i];
        if (B.neg)
            ans = -ans;
        return ans;
    }
    // reduce B from the highest bits of A (with the same width)

    // floor division
    // 牛顿迭代除法
    int2048 operator/(int2048 A, int2048 B)
    {
        if (B == int2048(0))
        {
            std::cout << "divide 0\n";
            exit(0);
        }
        if (B.neg)
            return (-A) / (-B);
        if (A.neg)
            return -((-A - 1) / B + 1);
        // 处理 A < B
        if (A < B)
            return int2048(0);
        int2048 oldA = A, oldB = B;
        int shift = 0;
        int n = A.len, m = B.len;

        if (A.len > 2 * B.len)
        {
            shift = A.len - 2 * B.len;
            A = MoveLeft(A, shift);
            B = MoveLeft(B, shift);
            // After movement, A.len = 2n-2m, B.len = n-m, then A.len <= 2*B.len
        }
        int2048 X = B.INV();
        int2048 Q = A * X;
        Q = MoveRight(Q, 2 * B.len);
        int2048 R = oldA - Q * oldB;
        while (R >= oldB)
        {
            Q = Q + 1;
            R = R - oldB;
        }
        while (R < 0)
        {
            Q = Q - 1;
            R = R + oldB;
        }
        return Q;
    }

    // using floor division we can guarantee that the remainder is non-negative
    int2048 operator%(int2048 A, const int2048 &B) { return A - A / B * B; }

    std::istream &operator>>(std::istream &is, int2048 &A)
    {
        std::string s;
        is >> s;
        A.read(s);
        return is;
    }

    std::ostream &operator<<(std::ostream &os, const int2048 &A)
    {
        int2048 B = A;
        B.print(os);
        return os;
    }

    bool operator==(const int2048 &A, const int2048 &B)
    {
        if (A.neg != B.neg)
            return false;
        return !comp_abs(A, B);
    }
    bool operator!=(const int2048 &A, const int2048 &B) { return !(A == B); }

    bool operator<(const int2048 &A, const int2048 &B)
    {
        // A<0,B>0 then A<B; A>0,B<0 then !(A<B)
        if (A.neg != B.neg)
            return A.neg;
        // if A<0,B<0, then A<B is equivalent to |A| > |B|
        if (A.neg)
            return comp_abs(A, B) == 1;
        return comp_abs(A, B) == -1;
    }

    bool operator>(const int2048 &A, const int2048 &B) { return B < A; }

    bool operator<=(const int2048 &A, const int2048 &B) { return !(A > B); }

    bool operator>=(const int2048 &A, const int2048 &B) { return !(A < B); }

    // doubling technique, for cases where the x,y is big but x/y is small
    int2048 div_special(const int2048 &A, const int2048 &B)
    {
        int2048 b = B, a = int2048(1);
        std::vector<int2048> p2B(1), p2(1);
        int cnt = 0;
        while (b <= A)
        {
            ++cnt;
            p2B.push_back(b);
            p2.push_back(a);
            a = a + a;
            b = b + b;
        }
        b = A;
        int2048 ans;
        while (cnt >= 1)
        {
            if (b >= p2B[cnt])
            {
                b -= p2B[cnt];
                ans += p2[cnt];
            }
            cnt--;
        }
        return ans;
    }

    int2048 int2048::INV()
    {
        if (len <= 10)
        {
            int2048 t = MoveLeft(int2048(1), len * 2);
            return div_special(t, *this);
        }
        int k = (len + 1) / 2 + 2;
        int2048 t = MoveRight(*this, len - k);
        int2048 res_t = t.INV();
        int2048 res = MoveLeft(2 * res_t, len - k) - MoveRight(*this * res_t * res_t, 2 * k);
        return res;
    }

    // only for constant divisions <= BASE
    int2048 operator/(int2048 A, const LL &B)
    {
        if (B == 0)
        {
            std::cout << "divisor is zero\n";
            exit(0);
        }
        if (B < 0)
            return (-A) / (-B);
        if (B >= int2048::BASE)
            return A / int2048(B);
        int2048 ans = A;
        LL rem = 0; // remainder
        for (int i = A.len - 1; ~i; --i)
        {
            LL nx = rem * int2048::BASE + A.num[i];
            ans.num[i] = nx / B;
            rem = nx % B;
        }
        // in this case we got A % B = -rem
        if (A.neg && rem > 0)
            ans -= int2048(1LL);
        ans.carry();
        return ans;
    }
    // A*BASE^n
    int2048 MoveLeft(const int2048 &A, int n)
    {
        int2048 ans;
        ans.num.clear();
        for (int i = 1; i <= n; ++i)
            ans.num.push_back(0);
        for (const int &x : A.num)
            ans.num.push_back(x);
        ans.len = ans.num.size();
        ans.neg = A.neg;
        return ans;
    }

    // assure that B < BASE;
    int2048 operator*(int2048 A, const LL &B)
    {
        if (B >= int2048::BASE)
            return A * (int2048)B;
        for (int i = 0; i < A.len; ++i)
            A.num[i] *= B;
        A.carry();
        return A;
    }

    int2048 &int2048::operator*=(const LL &B)
    {
        *this = *this * B;
        return *this;
    }

    int2048 MoveRight(const int2048 &A, int n)
    {
        if (n >= A.len)
            return 0;
        int2048 ans;
        ans.num.clear();
        ans.neg = A.neg;
        ans.len = A.len - n;
        for (int i = n; i < A.len; ++i)
            ans.num.push_back(A.num[i]);
        return ans;
    }

} // namespace sjtu
