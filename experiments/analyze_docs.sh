#!/bin/bash

echo "Analyzing documentation files in docs/"
echo "======================================="
echo

cd /tmp/gh-issue-solver-1765740913088

for file in docs/*.md; do
    if [ -f "$file" ]; then
        echo "File: $(basename "$file")"
        echo "First 3 lines:"
        head -3 "$file"

        # Check if file contains Russian characters
        if grep -q '[А-Яа-яЁё]' "$file"; then
            echo "Language: Contains Russian"
        else
            echo "Language: English (no Russian characters detected)"
        fi

        echo "---"
        echo
    fi
done
