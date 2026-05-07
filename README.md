# Annex Technologies Limited — Technical Assessment (ASHER NJOROGE)

Solutions to the three-question technical assessment covering C, SQL, and C++.

---

## Folder Structure

```
Annex_Assessment_<YourName>/
├── pth_factor.c        # Q1 — P-th Factor (C)
├── top_students.sql    # Q2 — Top Scoring Students (SQL)
├── top_articles.cpp    # Q3 — Top Articles API (C++)
└── README.md
```

---

## Q1 — P-th Factor (`pth_factor.c`) · C

**Problem:** Given two integers `n` and `p`, return the p-th smallest factor of `n`. Return `0` if `n` has fewer than `p` factors.

**My approach:** Rather than looping all the way to `n` to collect factors, I only iterate up to `√n`. For every `i` that divides `n`, both `i` and `n/i` are factors — so you get two factors per iteration. I collect them all, sort in ascending order, then index into the list. This keeps the solution at O(√n), which is what makes it comfortable even at the constraint ceiling of n = 10¹⁵.

**Build & run:**
```bash
# Linux / macOS
gcc -std=c11 -O2 -o pth_factor pth_factor.c -lm
./pth_factor

# Windows (MSYS2)
gcc -std=c11 -O2 -o pth_factor pth_factor.c -lm
./pth_factor
```

**Quick check:**
```
Input:  n = 10, p = 3
Output: 5          ✓   (factors of 10 are {1, 2, 5, 10})
```

---

## Q2 — Top Scoring Students (`top_students.sql`) · SQL

**Problem:** Retrieve the ID and NAME of the three highest-scoring students. Ties in score are broken by ID ascending.

**My approach:** Straightforward `ORDER BY score DESC, id ASC` with a `LIMIT 3`. The tie-breaker on ID ensures deterministic results when two students share the same score — as is the case with Dick and Jerry both sitting at 85.0 in the sample data.

**Run the query:**
```bash
# MySQL
mysql -u your_user -p your_database < top_students.sql
```

**Expected output for sample data:**
```
ID   NAME
6    Sid
7    Tom
4    Dick
```

---

## Q3 — Top Articles (`top_articles.cpp`) · C++

**Problem:** Hit the HackerRank mock Articles API, pull every page, and return the names of the top `limit` articles ranked by comment count.

**My approach:** I fetch page 1 first to read `total_pages`, then loop through the remaining pages. For each article object I apply the name-resolution rule from the spec (`title` → `story_title` → skip), treating any `null` or missing `num_comments` as zero. After collecting everything, I sort with a two-key comparator: comment count descending, then article name descending as the tie-breaker. No third-party JSON library — parsing is handled with lightweight string helpers to keep the build simple.

**Dependencies:** libcurl

**Build & run:**
```bash
# Linux
sudo apt-get install libcurl4-openssl-dev
g++ -std=c++17 -O2 -o top_articles top_articles.cpp -lcurl
./top_articles

# macOS
brew install curl
g++ -std=c++17 -O2 -o top_articles top_articles.cpp -lcurl
./top_articles

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-curl
g++ -std=c++17 -O2 -o top_articles top_articles.cpp -lcurl
./top_articles
```

**Quick check:**
```
Input:  limit = 2
Output:
UK votes to leave EU
F.C.C. Repeals Net Neutrality Rules
```

#### SSL on Windows

If you hit an SSL certificate error on Windows, copy the CA bundle from your MSYS2 install into the same folder as the `.exe`:

```bash
cp C:/msys64/mingw64/ssl/certs/ca-bundle.crt .
```

Then point curl to it in `httpGet()`:
```cpp
curl_easy_setopt(curl, CURLOPT_CAINFO, "ca-bundle.crt");
```

This avoids disabling SSL verification entirely, which matters if you're running this against any real API down the line.
