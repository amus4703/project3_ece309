# Project 3 Design Log

**Author:** Ayoub Musa (review and personalize before submission)  
**P2 code used:** My own Project 2 implementations of `Conversation` and `SentinelScanner`.  
**Tools used:** ChatGPT (OpenAI) assisted with integrating the P2 files, implementing the Project 3 registry, path resolver, and stack evaluator, extending the tests, running the build and test commands, and drafting this design log. I will review and edit the code and this explanation before submission.

## 1. Hash table

I chose **separate chaining** instead of linear probing for the tool registry. Each bucket is a `std::vector` containing `std::unique_ptr<Tool>` entries. Chaining makes collisions straightforward: entries with the same bucket index are stored together, and lookup only examines that bucket. It also avoids the possibility of a failed lookup looping forever through a full table. Linear probing could use less allocation overhead, but I preferred the clearer collision logic for this project.

The hash function is a 64-bit FNV-1a-style string hash. It starts from a fixed basis, XORs every byte into the hash, and multiplies by a fixed prime after each byte. This makes the result depend on both the characters and their order. The hash function returns the full-width result; the registry separately calculates `hash(name) % bucket_count()` to choose a bucket. A simple hash based on the first character and the string length would produce severe clustering for names like `tool_123` and `tool_321`.

I start with 16 buckets and use a maximum load factor of **0.75**. Before adding a new entry, the registry checks whether the insertion would cross this threshold. If so, it doubles the number of buckets and redistributes the existing pointers using their names and the new bucket count. A duplicate name is rejected before resizing, so duplicates do not change the table. Each ordinary insertion costs expected constant time. Resizing costs O(n) for n entries, but the number of inserts between successive doublings grows with n; spreading the occasional rehash cost over these inserts gives **amortized O(1)** insertion.

In the required 10,000-name test, the table had **16,384 buckets**, a **0.610352 load factor**, and a **maximum chain length of 6**. By contrast, the deliberately weak `s[0] + s.size()` hash would have created a **longest chain of 9,000** at the same bucket count. The real implementation stays comfortably below the required maximum probe length of 64. These results show why distributing similar names matters.

## 2. Pointer stability

The registry owns each tool through a `std::unique_ptr<Tool>`. A lookup returns a non-owning raw `Tool*`, not a copy of the object. During rehashing, the `unique_ptr` values move between bucket vectors, but the `Tool` instances remain at their original heap addresses. Therefore, a pointer obtained before a resize still refers to the same tool afterward. The registry remains responsible for destroying each tool exactly once. This distinction matters for `find()`.

## 3. Calculator trace

The evaluator uses one stack for numbers and another for operators and left parentheses. For `2 * (3 + 4) - 5`, reading `2` puts `[2]` in the values stack. Reading `*` puts `[*]` in the operator stack. Reading `(` makes the operators `[* , (]`; then `3`, `+`, and `4` produce values `[2, 3, 4]` and operators `[* , (, +]`. At `)`, the evaluator pops `+`, combines 3 and 4 into 7, and removes the matching `(`. The stacks now contain values `[2, 7]` and operators `[*]`. On reading `-`, multiplication has higher precedence, so the evaluator computes 2 times 7 and then pushes `-`. Reading `5` adds it to the values stack. At the end, applying subtraction yields **9**. Popping the right operand before the left is essential for subtraction and division. The algorithm uses loops instead of recursion, so deeply nested parentheses do not consume recursive call frames.

## 4. Errors as values

A failed tool request is a normal outcome, not necessarily a broken program. A model might request a missing file, try to leave the workspace, or divide by zero. Returning `false` with a reason, or returning `ToolResult::failure`, lets the harness include that failure in the conversation so the model can respond appropriately. Exceptions are more appropriate for unexpected programming failures, such as using a JSON accessor with the wrong value type. Keeping normal input errors as values makes the interface predictable and keeps a single bad request from interrupting the tool loop.

## 5. What I would design differently

If I revisited the implementation, I would add a dedicated small `Bucket` abstraction and explicit metrics for total collisions and average successful lookup length. Those measurements would make it easier to compare chaining against linear probing using the same test inputs. I would keep the existing `ToolRegistry` public interface unchanged because it already separates ownership, lookup, iteration, and instrumentation cleanly.
