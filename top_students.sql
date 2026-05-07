-- Top Scoring Students
-- ---------------------------------------------------------------
-- Retrieve the ID and NAME of the three highest-scoring students.
-- Tie-breaking rule: higher score first; equal score → lower ID first.
-- ---------------------------------------------------------------

SELECT
    ID,
    NAME
FROM
    STUDENT
ORDER BY
    SCORE DESC,   -- primary   : highest score first
    ID    ASC     -- secondary : lowest ID first (tie-break)
LIMIT 3;
