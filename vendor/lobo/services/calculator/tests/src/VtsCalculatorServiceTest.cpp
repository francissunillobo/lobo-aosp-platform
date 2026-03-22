// Copyright (C) 2024 Lobo Platform
// SPDX-License-Identifier: Apache-2.0

#define LOG_TAG "VtsCalculatorServiceTest"

#include <atomic>
#include <climits>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <aidl/com/lobo/platform/calculator/ICalculatorService.h>

using aidl::com::lobo::platform::calculator::ICalculatorService;

static constexpr const char* kServiceName =
        "com.lobo.platform.calculator.ICalculatorService";

class CalculatorServiceTest : public ::testing::Test {
public:
    static std::shared_ptr<ICalculatorService> sService;

    static void SetUpTestSuite() {
        ABinderProcess_setThreadPoolMaxThreadCount(0);
        ABinderProcess_startThreadPool();

        ndk::SpAIBinder binder(AServiceManager_waitForService(kServiceName));
        if (binder.get() != nullptr) {
            sService = ICalculatorService::fromBinder(binder);
        }
    }

protected:
    void SetUp() override {
        if (sService == nullptr) {
            GTEST_SKIP() << "calculatord not running — is it installed and started?";
        }
    }
};

std::shared_ptr<ICalculatorService> CalculatorServiceTest::sService = nullptr;

#define ASSERT_BINDER_OK(status)                                               \
    ASSERT_TRUE((status).isOk())                                               \
        << "Binder call failed: " << (status).getDescription()

// Service availability
TEST_F(CalculatorServiceTest, ServiceRegistered) {
    ndk::SpAIBinder binder(AServiceManager_checkService(kServiceName));
    ASSERT_NE(binder.get(), nullptr) << "Service not found under: " << kServiceName;
}

// add
TEST_F(CalculatorServiceTest, Add_TwoPositives) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->add(10, 3, &result));
    EXPECT_EQ(result, 13);
}

TEST_F(CalculatorServiceTest, Add_TwoNegatives) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->add(-5, -3, &result));
    EXPECT_EQ(result, -8);
}

TEST_F(CalculatorServiceTest, Add_WithZero) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->add(42, 0, &result));
    EXPECT_EQ(result, 42);
}

// subtract
TEST_F(CalculatorServiceTest, Subtract_PositiveResult) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->subtract(10, 3, &result));
    EXPECT_EQ(result, 7);
}

TEST_F(CalculatorServiceTest, Subtract_NegativeResult) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->subtract(3, 10, &result));
    EXPECT_EQ(result, -7);
}

// multiply
TEST_F(CalculatorServiceTest, Multiply_TwoPositives) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->multiply(4, 5, &result));
    EXPECT_EQ(result, 20);
}

TEST_F(CalculatorServiceTest, Multiply_ByZero) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->multiply(999, 0, &result));
    EXPECT_EQ(result, 0);
}

// divide
TEST_F(CalculatorServiceTest, Divide_ExactDivision) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->divide(10, 2, &result));
    EXPECT_EQ(result, 5);
}

TEST_F(CalculatorServiceTest, Divide_IntegerTruncation) {
    int32_t result = 0;
    ASSERT_BINDER_OK(sService->divide(7, 2, &result));
    EXPECT_EQ(result, 3);
}

TEST_F(CalculatorServiceTest, Divide_ByZero_ReturnsErrorStatus) {
    int32_t result = 0;
    ndk::ScopedAStatus status = sService->divide(10, 0, &result);
    EXPECT_FALSE(status.isOk());
}

TEST_F(CalculatorServiceTest, Divide_ByZero_ErrorCode_IsOne) {
    int32_t result = 0;
    ndk::ScopedAStatus status = sService->divide(10, 0, &result);
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(status.getExceptionCode(), EX_SERVICE_SPECIFIC);
    EXPECT_EQ(status.getServiceSpecificError(), ICalculatorService::ERROR_DIVIDE_BY_ZERO);
}

TEST_F(CalculatorServiceTest, ErrorConstant_DivideByZero_IsOne) {
    EXPECT_EQ(ICalculatorService::ERROR_DIVIDE_BY_ZERO, 1);
}

// Concurrency
TEST_F(CalculatorServiceTest, ConcurrentCalls_NoDataRace) {
    static constexpr int kThreads        = 8;
    static constexpr int kCallsPerThread = 100;
    std::atomic<int> failures{0};
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int t = 0; t < kThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < kCallsPerThread; ++i) {
                int32_t result = 0;
                ndk::ScopedAStatus s = sService->add(t, i, &result);
                if (!s.isOk() || result != t + i) failures++;
            }
        });
    }
    for (auto& th : threads) th.join();
    EXPECT_EQ(failures.load(), 0);
}

TEST_F(CalculatorServiceTest, RepeatedCalls_ServiceStable) {
    for (int i = 0; i < 1000; ++i) {
        int32_t result = 0;
        ASSERT_BINDER_OK(sService->add(i, 1, &result));
        EXPECT_EQ(result, i + 1);
    }
}
