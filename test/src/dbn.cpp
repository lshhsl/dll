//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include <deque>

#include "catch.hpp"

#include "dll/dbn.hpp"

#include "mnist/mnist_reader.hpp"
#include "mnist/mnist_utils.hpp"

TEST_CASE( "dbn/mnist_1", "dbn::simple" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
        , dll::batch_size<50>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(500);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

TEST_CASE( "dbn/mnist_2", "dbn::containers" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
        , dll::batch_size<50>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(500);

    REQUIRE(!dataset.training_images.empty());
    dataset.training_images.resize(200);
    dataset.training_labels.resize(200);

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 5);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 5);

    REQUIRE(error < 5e-2);
}

TEST_CASE( "dbn/mnist_3", "dbn::labels" ) {
    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(1000);
    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    typedef dll::dbn_desc<
        dll::dbn_label_layers<
        dll::rbm_desc<28 * 28, 200, dll::batch_size<50>, dll::init_weights, dll::momentum>::rbm_t,
        dll::rbm_desc<200, 300, dll::batch_size<50>, dll::momentum>::rbm_t,
        dll::rbm_desc<310, 500, dll::batch_size<50>, dll::momentum>::rbm_t>
        , dll::batch_size<10>
    >::dbn_t dbn_t;

    auto dbn = std::make_unique<dbn_t>();

    dbn->train_with_labels(dataset.training_images, dataset.training_labels, 10, 10);

    auto error = dll::test_set(dbn, dataset.training_images, dataset.training_labels, dll::label_predictor());
    std::cout << "test_error:" << error << std::endl;
    REQUIRE(error < 0.3);
}

TEST_CASE( "dbn/mnist_6", "dbn::cg_gaussian" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 200, dll::momentum, dll::batch_size<25>, dll::visible<dll::unit_type::GAUSSIAN>>::rbm_t,
        dll::rbm_desc<200, 500, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<500, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
        , dll::batch_size<50>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(1000);

    REQUIRE(!dataset.training_images.empty());

    mnist::normalize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

//This test should not perform well, but should not fail
TEST_CASE( "dbn/mnist_8", "dbn::cg_relu" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::RELU>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
        , dll::batch_size<50>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(200);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10);

    REQUIRE(std::isfinite(error));

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;
}

TEST_CASE( "dbn/mnist_15", "dbn::parallel" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::parallel_mode, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::parallel_mode, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::parallel_mode, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
        , dll::batch_size<50>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(500);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(dataset.training_images, dataset.training_labels, 10);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);
}

TEST_CASE( "dbn/mnist_17", "dbn::memory" ) {
    typedef dll::dbn_desc<
        dll::dbn_layers<
        dll::rbm_desc<28 * 28, 100, dll::momentum, dll::batch_size<25>, dll::init_weights>::rbm_t,
        dll::rbm_desc<100, 200, dll::momentum, dll::batch_size<25>>::rbm_t,
        dll::rbm_desc<200, 10, dll::momentum, dll::batch_size<25>, dll::hidden<dll::unit_type::SOFTMAX>>::rbm_t>
    , dll::memory
    , dll::batch_size<50>
    , dll::big_batch_size<3>
    >::dbn_t dbn_t;

    auto dataset = mnist::read_dataset_direct<std::vector, etl::dyn_matrix<float, 1>>(1078);

    REQUIRE(!dataset.training_images.empty());

    mnist::binarize_dataset(dataset);

    auto dbn = std::make_unique<dbn_t>();

    dbn->pretrain(dataset.training_images, 20);
    auto error = dbn->fine_tune(
        dataset.training_images.begin(), dataset.training_images.end(),
        dataset.training_labels.begin(), dataset.training_labels.end(),
        10);

    REQUIRE(error < 5e-2);

    auto test_error = dll::test_set(dbn, dataset.test_images, dataset.test_labels, dll::predictor());

    std::cout << "test_error:" << test_error << std::endl;

    REQUIRE(test_error < 0.2);

    //Mostly here to ensure compilation
    auto out = dbn->prepare_one_output();
    REQUIRE(out.size() > 0);
}