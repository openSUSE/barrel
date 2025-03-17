

pair<string, string>
run_and_capture(int argc, char** argv, barrel::Testsuite* testsuite)
{
    ostringstream buffer1;
    ostringstream buffer2;

    streambuf* old1 = cout.rdbuf(buffer1.rdbuf());
    streambuf* old2 = cerr.rdbuf(buffer2.rdbuf());

    handle(argc, argv, testsuite);

    cout.rdbuf(old1);
    cerr.rdbuf(old2);

    return make_pair(buffer1.str(), buffer2.str());
}
