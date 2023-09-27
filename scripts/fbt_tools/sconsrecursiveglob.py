import SCons
from fbt.util import GLOB_FILE_EXCLUSION
from SCons.Script import Flatten
from SCons.Node.FS import has_glob_magic


def GlobRecursive(env, pattern, node=".", exclude=[]):
    exclude = list(set(Flatten(exclude) + GLOB_FILE_EXCLUSION))
    # print(f"Starting glob for {pattern} from {node} (exclude: {exclude})")
    results = []
    if isinstance(node, str):
        node = env.Dir(node)
    if has_glob_magic(pattern):
        for f in node.glob("*", source=True, exclude=exclude):
            if isinstance(f, SCons.Node.FS.Dir):
                results += env.GlobRecursive(pattern, f, exclude)
        results += node.glob(
            pattern,
            source=True,
            exclude=exclude,
        )
    elif (file_node := node.File(pattern)).rexists():
        results.append(file_node)
    # print(f"Glob result for {pattern} from {node}: {results}")
    return results


def generate(env):
    env.AddMethod(GlobRecursive)


def exists(env):
    return True
