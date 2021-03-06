/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <stdlib.h>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WGridLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTreeView.h>
#include <Wt/WVBoxLayout.h>


#include "GitModel.h"
#include "../wt-homepage/SourceView.h"

using namespace Wt;

/**
 * \defgroup gitmodelexample Git model example
 */
/*@{*/

/*! \class GitViewApplication
 *  \brief A simple application to navigate a git repository.
 *
 * This examples demonstrates how to use the custom model use GitModel
 * with a WTreeView.
 */
class GitViewApplication : public WApplication
{
public:
  /*! \brief Constructor.
   */
  GitViewApplication(const WEnvironment& env) 
    : WApplication(env)
  {
    useStyleSheet("gitview.css");
    setTitle("Git model example");

    const char *gitRepo = getenv("GITVIEW_REPOSITORY_PATH");

    auto grid
        = cpp14::make_unique<WGridLayout>();
    grid->addWidget(cpp14::make_unique<WText>("Git repository path:"), 0, 0);

    repositoryEdit_ = grid->addWidget(cpp14::make_unique<WLineEdit>(gitRepo ? gitRepo : ""),
                                      0, 1, AlignmentFlag::Left);
    repositoryError_ = grid->addWidget(cpp14::make_unique<WText>(), 0, 2);

    grid->addWidget(cpp14::make_unique<WText>("Revision:"), 1, 0);

    revisionEdit_ = grid->addWidget(cpp14::make_unique<WLineEdit>("master"),
                                    1, 1, AlignmentFlag::Left);
    revisionError_ = grid->addWidget(cpp14::make_unique<WText>(), 1, 2);

    repositoryEdit_->setTextSize(30);
    revisionEdit_->setTextSize(20);
    repositoryError_->setStyleClass("error-msg");
    revisionError_->setStyleClass("error-msg");

    repositoryEdit_->enterPressed()
      .connect(this, &GitViewApplication::loadGitModel);
    revisionEdit_->enterPressed()
      .connect(this, &GitViewApplication::loadGitModel);

    auto button = grid->addWidget(cpp14::make_unique<WPushButton>("Load"),
                                  2, 0, AlignmentFlag::Left);
    button->clicked().connect(this, &GitViewApplication::loadGitModel);

    auto gitView = cpp14::make_unique<WTreeView>();
    gitView_ = gitView.get();
    gitView_->resize(300, WLength::Auto);
    gitView_->setSortingEnabled(false);

    gitModel_
        = std::make_shared<GitModel>();
    gitView_->setModel(gitModel_);
    gitView_->setSelectionMode(SelectionMode::Single);
    gitView_->selectionChanged().connect(this, &GitViewApplication::showFile);

    auto sourceView
        = cpp14::make_unique<SourceView>(ItemDataRole::Display,
                                         GitModel::ContentsRole, GitModel::FilePathRole);
    sourceView_ = sourceView.get();
    sourceView_->setStyleClass("source-view");

    /* FIXME: adding a gridlayout to a box layout */
    if (environment().javaScript()) {
      /*
       * We have JavaScript: We can use layout managers so everything will
       * always fit nicely in the window.
       */
      auto topLayout = root()->setLayout(cpp14::make_unique<WVBoxLayout>());
      root()->setStyleClass("maindiv");
      topLayout->addLayout(std::move(grid),0);

      auto gitLayout = cpp14::make_unique<WHBoxLayout>();
      gitLayout->addWidget(std::move(gitView),0);
      gitLayout->addWidget(std::move(sourceView),1);
      topLayout->addLayout(std::move(gitLayout),1);
    } else {
      /*
       * No JavaScript: let's make the best of the situation using regular
       * CSS-based layout
       */
      root()->setStyleClass("maindiv");
      auto top
          = cpp14::make_unique<WContainerWidget>();
      top->setLayout(std::move(grid));
      root()->addWidget(std::move(top));
      root()->addWidget(std::move(gitView));
      gitView_->setFloatSide(Side::Left);
      gitView_->setMargin(6);
      root()->addWidget(std::move(sourceView));
      sourceView_->setMargin(6);
    }
  }

private:
  WLineEdit  *repositoryEdit_, *revisionEdit_;
  WText      *repositoryError_, *revisionError_;
  std::shared_ptr<GitModel>   gitModel_;
  WTreeView  *gitView_;
  SourceView *sourceView_;

  /*! \brief Change repository and/or revision
   */
  void loadGitModel() {
    sourceView_->setIndex(WModelIndex());
    repositoryError_->setText("");
    revisionError_->setText("");
    try {
      gitModel_->setRepositoryPath(repositoryEdit_->text().toUTF8());
      try {
	gitModel_->loadRevision(revisionEdit_->text().toUTF8());
      } catch (const Git::Exception& e) {
	revisionError_->setText(e.what());
      }
    } catch (const Git::Exception& e) {
      repositoryError_->setText(e.what());
    }
  }

  /*! \brief Displayed the currently selected file.
   */
  void showFile() {
    if (gitView_->selectedIndexes().empty())
      return;

    WModelIndex selected = *gitView_->selectedIndexes().begin();
    sourceView_->setIndex(selected);
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<GitViewApplication>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}

/*@}*/
