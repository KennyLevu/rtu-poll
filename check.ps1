Get-Content -Path "Makefile" | ForEach-Object {
    # Replace non-printable characters with their escaped representation
    $_ -replace "[^\x20-\x7E]", { '0x{0:X}' -f [byte]$_ }

    # Replace tabs with '^I'
    $_ -replace "`t", "^I"

    # Append end-of-line characters ('$' represents the end of each line)
    $_ + '$'
}
