# Build

* `-p` Create projects folder
* `-vh` High verbosity

```shell
gprbuild -p -vh
```

# Run

## Live video

```shell
./test_grab | ./test_render
```

## Save video

* `[Period]` How many seconds of video stored in each file

```shell
./test_grab | ./test_save [Period]
```

## Play video

* `[Filename]` The filename to play.

```shell
./test_render < [Filename]
```
