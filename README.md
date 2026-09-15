# Connected Word Recognition

A C++17 demonstration of one-stage dynamic programming for connected-word recognition. The program matches an input sequence against word templates, combining time alignment and word-boundary detection to recover a minimum-distance word sequence.

The example uses **synthetic feature vectors**. It demonstrates the matching algorithm; it does not transcribe audio recordings.

## Repository structure

```text
connected-word-recognition/
├── src/
│   └── connected_word_recognition.cpp
├── docs/
│   ├── report.pdf
│   └── presentation.pdf
├── .gitattributes
├── .gitignore
└── README.md
```

- [Source code](src/connected_word_recognition.cpp): implementation, sample input, and built-in checks.
- [Project report](docs/report.pdf): project explanation and implementation details.
- [Presentation](docs/presentation.pdf): original academic presentation. Some implementation examples predate the corrected code; use this README and the source for current behavior.

## Build and run

Requires a C++17 compiler such as Clang or GCC. No third-party libraries are needed.

Run these commands from the repository root on macOS or Linux:

```sh
mkdir -p build
c++ -std=c++17 -Wall -Wextra -Wpedantic src/connected_word_recognition.cpp -o build/connected_word_recognition
./build/connected_word_recognition
```

Expected output:

```text
Recognized words: ONE TWO
Total distance: 0
All demonstration checks passed.
```

## Sample input

Each frame contains two numeric features. The labels ONE and TWO name synthetic templates, not recorded words.

| Pattern | Feature frames |
| --- | --- |
| ONE | `(0,0), (1,0), (2,0)` |
| TWO | `(8,0), (9,0)` |
| Input | `(0,0), (1,0), (1,0), (2,0), (8,0), (9,0)` |

The extra `(1,0)` input frame demonstrates time stretching. The best alignment matches ONE followed by TWO with zero accumulated distance.

The sample is defined in `main()`. To try another example, change `vocabulary` and `input` and adjust the corresponding expected checks. There are no command-line input arguments or file loaders.

## How the algorithm works

1. Compute Euclidean distances between input and template feature vectors.
2. Store the best accumulated cost for each input frame, template, and template frame. Unreachable states start at infinity.
3. Within a template, consider vertical, diagonal, and horizontal predecessors while preserving frame order.
4. At the first frame of a template, consider staying there or starting a word after any completed template at the previous input frame. Cache the best completed template once per input frame.
5. Select the cheapest completed template at the final input frame and follow saved predecessors and word-start markers to recover the words.

The implementation accepts templates of different lengths. All frames must have the same nonzero feature dimension and finite values. Empty inputs and templates are rejected.

### Complexity

Let `N` be the number of input frames, `S` the sum of all template lengths, and `F` the feature dimension.

- Time: **O(N × S × F)**, or **O(N × S)** for a fixed feature dimension.
- DP and traceback storage: **O(N × S)**, excluding input and template storage.

## Validation

Running the executable also runs these built-in checks. A failed check prints an error and exits with a nonzero status.

| Check | Expected behavior |
| --- | --- |
| Time-stretched connected sequence | ONE TWO with cost 0 |
| Repeated word boundaries | ONE ONE |
| Compressed alignment using a horizontal move | Cost 1 |
| Fractional feature distance | Cost 0.5 |
| Mismatched feature dimensions | Rejected |

These small examples check specific behaviors; they are not an exhaustive proof of correctness or a speech-accuracy benchmark.

## Academic context

Developed for **CS354 Advanced Algorithms**, based on *The Use of a One-Stage Dynamic Programming Algorithm for Connected Word Recognition*.

Project team: Sai Kiran Sajja, Ganesh Yadava, and Dhruv Aggarwal. Course guidance: Dr. Manish Kumar Bajpai.

Figures and comparison results reproduced in the academic documents belong to their cited sources. The paper's comparison results are not benchmarks of this implementation.
