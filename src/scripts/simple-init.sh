
mount chardev / chr sys y
mount ramfile / ramfile sys y
mount kbd / kbd sys y

setstdin /chr/COM0
setstdout /chr/COM0
setstderr /chr/COM0

exec sh

