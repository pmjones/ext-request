#!/usr/bin/env php
<?php
// modified from https://raw.githubusercontent.com/m6w6/ext-http/master/scripts/check_package-xml.php
ini_set("display_errors", true);
$xml = simplexml_load_file("./package.xml");
$xml_files = xmllist($xml->contents[0]);
$dirs = ["."];
$exit = 0;
while ($dir = array_shift($dirs)) {
    foreach (dirlist($dir) as $file) {
        if (is_git_ignored($file)) {
            continue;
        }
        if (is_otherwise_ignored($file)) {
            continue;
        }
        if (!is_dir($file)) {
            if (!in_array($file, $xml_files)) {
                echo "Missing file $file\n";
                $exit = 1;
            }
        } else {
            $base = basename($file);
            if ($base{0} !== ".") {
                array_push($dirs, $file);
            }
        }
    }
}
foreach ($xml_files as $file) {
    if (!file_exists($file)) {
        echo "Extraneous file $file\n";
        $exit = 1;
    }
}
exit($exit);

###

function error($fmt) {
    trigger_error(call_user_func_array("sprintf", func_get_args()));
}

function is_git_ignored($file) {
    static $gitignore, $gitmodules;

    if (!isset($gitmodules)) {
        if (is_readable("./.gitmodules")) {
            $gitmodules = explode("\n", `git submodule status | awk '{printf$2}'`);
        } else {
            $gitmodules = false;
        }
    }
    if (!isset($gitignore)) {
        if (is_readable("./.gitignore")) {
            $ignore_submodules = $gitmodules ? " ! -path './".implode("/*' ! -path './", $gitmodules)."/*'" : "";
            $gitignore = explode("\n", `find . $ignore_submodules | git check-ignore --stdin`);
        } else {
            $gitignore = false;
        }
    }
    if ($gitignore) {
        if (in_array($file, $gitignore)) {
            return true;
        }
    }
    if ($gitmodules) {
        foreach ($gitmodules as $module) {
            if (fnmatch("./$module/*", $file)) {
                return true;
            }
        }
    }
    return false;
}

function is_otherwise_ignored($file) {
    $ignored = [
        "./package.xml", "./package2.xml", "./.travis.yml", "./.editorconfig",
        "./run-tests.sh", "./reflect.php", "./clean.sh", "./prepackage.php",
        "./appveyor.yml"
    ];
    return fnmatch("./.git*", $file)
        || in_array($file, $ignored, true);
}

function xmllist(SimpleXmlElement $dir, $p = ".", &$a = null) {
    settype($a, "array");
    $p = trim($p, "/") . "/" . trim($dir["name"], "/") . "/";
    foreach ($dir as $file) {
        switch ($file->getName()) {
            case "dir":
                xmllist($file, $p, $a);
                break;
            case "file":
                $a[] = sprintf("%s/%s", trim($p, "/"), trim($file["name"]));
                break;
            default:
                trigger_error("Unknown content type: " . $file->getName());
                exit(1);
        }
    }
    return $a;
}

function dirlist($dir, $p = null) {
    $p = implode("/", array_filter([trim($p, "/"), trim($dir, "/")]));
    foreach (scandir($p) as $file) {
        yield $p."/".$file;
    }
}
