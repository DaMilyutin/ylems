# Elements

Some specific elements implementation.

## Yield types

### yield

Wrapper to make something yieldable.

### zip

### iota

runing finite or "infinite" sequence generated as arithmetic progression.

## Link types

### take

take at most given number of elements before stopping iteration


### drop

drop at least given number of elements before stopping iteration

### take_while

take elements while condition id fulfiled 

### drop_while

drop elements while condition id fulfiled 

### filter

pass only those elements for which condition is fuliled

### transform

transform elements and pass further

### transform_or

pass only those elements for which condition is fuliled
then transform them and pass further

### tee

TODO: attach intermediary sink

## Sink types

### fork

TODO: make single sink from tuple of sinks

### push_back

TODO: push back elements to collection

### insert

TODO: insert elements to collection

### collect

TODO: collect elements

### stream_out

TODO: send elements to output stream