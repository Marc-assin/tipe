import math

INPUT_FILE = "knot_gen_cut.c"
LINES_PER_SLIDE = 20

with open(INPUT_FILE, "r", encoding="utf-8", errors="ignore") as f:
    n_lines = sum(1 for _ in f)

n_slides = math.ceil(n_lines / LINES_PER_SLIDE)

for slide in range(n_slides):
    start = slide * LINES_PER_SLIDE + 1
    end = min((slide + 1) * LINES_PER_SLIDE, n_lines)

    print(r"\begin{frame}[fragile]")
    print(rf"\frametitle{{Code ({slide+1}/{n_slides})}}")
    print()
    print(
        rf"\lstinputlisting[language=C,firstline={start},lastline={end}]{{{INPUT_FILE}}}"
    )
    print()
    print(r"\end{frame}")
    print()