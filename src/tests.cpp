#include "tests.h"
#include "summarizer.h"
#include "settings.h"
#include "core/regex.h"

#define test(name,assertion,expected) if((assertion)!=(expected)) {\
    char error[1024];\
    snprintf(error, 1024, "test '%s' failed at %s:%d", name, __FILE__, __LINE__);\
    throw TesterException(error);\
    }

LogstalgiaTester::LogstalgiaTester()
{

}

void LogstalgiaTester::runTests() {

    FXFont font = fontmanager.grab("FreeMonoBold.ttf", settings.font_size);

    display.width  = 1024;
    display.height = 768;

    int abbreviation_depth = -1;
    int percent = 50;
    float update_rate = 1.0f;

    Summarizer* html_summarizer = 0;
    std::string html_title = "HTML";
    std::string html_regex = "\\.html?$";

    try {
        html_summarizer = new Summarizer(font, percent, abbreviation_depth, update_rate, html_regex, html_title);
    } catch(RegexCompilationException&) {

    }

    html_summarizer->addDelimiter('/');
    html_summarizer->setSize(0, 0, 0);

    test("html summarizer created", html_summarizer != 0, true);
    test("'/' is a delimiter",      html_summarizer->isDelimiter('/'), true);
    test("expected title",          html_summarizer->getTitle(), html_title);
    test("html summarizer accepts index.html", html_summarizer->supportedString("/index.html"), true);
    test("html summarizer rejects cat.jpg", html_summarizer->supportedString("/images/cat.jpg"), false);

    Summarizer* image_summarizer = 0;
    std::string image_regex = "\\.(jpe?g|png)$";
    std::string image_title = "Images";

    try {
        image_summarizer = new Summarizer(font, percent, abbreviation_depth, update_rate, image_regex, image_title);
    } catch(RegexCompilationException&) {

    }

    test("image summarizer created", image_summarizer != 0, true);

    image_summarizer->addDelimiter('/');
    image_summarizer->setSize(0, 0, 0);

    test("'/' is a delimiter",            image_summarizer->isDelimiter('/'), true);
    test("expected title",                image_summarizer->getTitle(), image_title);
    test("expected abbreivation depth",   image_summarizer->getAbbreviationDepth(), abbreviation_depth);

    test("image summarizer accepts cat.jpg",    image_summarizer->supportedString("/images/cat.jpg"), true);
    test("image summarizer accepts cat.png",    image_summarizer->supportedString("/images/cat.png"), true);
    test("image summarizer rejects index.html", image_summarizer->supportedString("/index.html"), false);

    image_summarizer->addString("/images/cat.jpg");
    image_summarizer->addString("/images/cat.png");
    image_summarizer->summarize();

    std::vector<std::string> summary;
    image_summarizer->getSummary(summary);

    test("summary size is 2", summary.size(), 2);
    test("summary[0] == /images/cat.jpg", summary[0], "/images/cat.jpg");
    test("summary[1] == /images/cat.png", summary[1], "/images/cat.png");

    image_summarizer->addString("/images/dog.jpg");
    image_summarizer->summarize();

    image_summarizer->getSummary(summary);
    test("summary size is 3", summary.size(), 3);
    test("summary[0] is /images/cat.jpg", summary[0], "/images/cat.jpg");
    test("summary[1] is /images/cat.png", summary[1], "/images/cat.png");
    test("summary[2] is /images/dog.jpg", summary[2], "/images/dog.jpg");

    test("best match string for dog.jpg", image_summarizer->getBestMatchStr("/images/dog.jpg"), "/images/dog.jpg");
    test("best match index for dog.jpg",  image_summarizer->getBestMatchIndex("/images/dog.jpg"), 2);

    // verify expected ref, word counts

    const SummNode* images_node = image_summarizer->getMatchingNode("/images/");
    test("/images/ node found", images_node != 0, true);
    test("/images/ refs is 3",  images_node->refs, 3);
    test("/images/ words is 3", images_node->words, 3);
    test("/images/ delimiters is 1", images_node->delimiters, 1);

    const SummNode* cat_node = image_summarizer->getMatchingNode("/images/cat");
    test("/images/cat node found", cat_node != 0, true);
    test("/images/cat refs is 2",  cat_node->refs, 2);
    test("/images/cat words is 2", cat_node->words, 2);
    test("/images/cat delimiters is 0", cat_node->delimiters, 0);

    const SummNode* cat_jpg_node = image_summarizer->getMatchingNode("/images/cat.jpg");
    test("/images/cat.jpg node found", cat_jpg_node != 0, true);
    test("/images/cat.jpg refs is 1",  cat_jpg_node->refs, 1);
    test("/images/cat.jpg words is 1", cat_jpg_node->words, 1);

    // NOTE: delimiters is count of delimiters at/after this node
    // so the 'g' at the end of the string will have no delimiters

    test("/images/cat.jpg delimiters is 0", cat_jpg_node->delimiters, 0);

    // additional request for cat.jpg
    image_summarizer->addString("/images/cat.jpg");

    cat_jpg_node = image_summarizer->getMatchingNode("/images/cat.jpg");
    test("/images/cat.jpg node found", cat_jpg_node != 0, true);
    test("/images/cat.jpg refs is 2",  cat_jpg_node->refs, 2);
    test("/images/cat.jpg words is 1", cat_jpg_node->words, 1);

    // check state after removing one reference to string /images/cat.jpg

    image_summarizer->removeString("/images/cat.jpg");

    cat_jpg_node = image_summarizer->getMatchingNode("/images/cat.jpg");
    test("/images/cat.jpg node found", cat_jpg_node != 0, true);
    test("/images/cat.jpg refs is 1",  cat_jpg_node->refs, 1);
    test("/images/cat.jpg words is 1", cat_jpg_node->words, 1);

    // removed all references to cat.jpg

    image_summarizer->removeString("/images/cat.jpg");

    cat_jpg_node = image_summarizer->getMatchingNode("/images/cat.jpg");
    test("/images/cat.jpg node no longer found", cat_jpg_node == 0, true);

    images_node = image_summarizer->getMatchingNode("/images/");
    test("/images/ refs is 2",  images_node->refs, 2);
    test("/images/ words is 2", images_node->words, 2);
    test("/images/ delimiters is 1", images_node->delimiters, 1);

    const SummNode* dog_jpg_node = image_summarizer->getMatchingNode("/images/dog.jpg");
    test("/images/dog.jpg node found", dog_jpg_node != 0, true);
    test("/images/dog.jpg refs is 1",  dog_jpg_node->refs, 1);
    test("/images/dog.jpg words is 1", dog_jpg_node->words, 1);
    test("/images/dog.jpg delimiters is 0", dog_jpg_node->delimiters, 0);

    image_summarizer->removeString("/images/dog.jpg");

    dog_jpg_node = image_summarizer->getMatchingNode("/images/dog.jpg");
    test("/images/dog.jpg no longer found", dog_jpg_node == 0, true);

    image_summarizer->removeString("/images/cat.png");

    images_node = image_summarizer->getMatchingNode("/images/");
    test("/images/ node no longer found", images_node == 0, true);
}
