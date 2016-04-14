# find all the files with suffix '_diff' and use the diff command to generate
# the patch file

: > openssl.patch
for dfile in $(find ./openssl -iname '*_diff'); do
  diff -u "${dfile%_diff}" "$dfile" >> openssl.patch
done

: > wolfssl.patch
for dfile in $(find ./wolfssl -iname '*_diff'); do
  diff -u "${dfile%_diff}" "$dfile" >> wolfssl.patch
done
