Set(FETCHCONTENT_QUIET FALSE)

#### Google FlatBuffer ####
FetchContent_Declare(
  flatbuffers
  GIT_REPOSITORY https://github.com/google/flatbuffers.git
  GIT_TAG v23.3.3
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(flatbuffers)
