// This code is for a blog post on FIRs: https://blog.demofox.org/2020/01/05/fir-audio-data-filters/

#include <stdio.h>
#include <vector>
#include <complex>

static const size_t c_numFrequencies = 100; // for frequency / phase response

static const float c_pi = 3.14159265359f;

typedef std::complex<float> complex;

complex Z(int delay, float angle)
{
    return std::polar(1.0f, float(delay)*angle);

    // the below is the same as the above
    /*
    complex ret;
    ret.real(cos(float(delay)*angle));
    ret.imag(sin(float(delay)*angle));
    return ret;
    */
}

// Not called, but shows how to apply an order 1 filter
void ApplyOrder1Filter(const std::vector<float>& input, std::vector<float>& output, float a0, float alpha1)
{
    output.resize(input.size());

    const float a1 = alpha1 * a0;

    float sample_n_1 = 0.0f;
    for (size_t index = 0, count = input.size(); index < count; ++index)
    {
        float sample_n = input[index];
        output[index] = sample_n * a0 + sample_n_1 * a1;
        sample_n_1 = sample_n;
    }
}

// Not called, but shows how to apply an order 2 filter
void ApplyOrder2Filter(const std::vector<float>& input, std::vector<float>& output, float a0, float alpha1, float alpha2)
{
    output.resize(input.size());

    const float a1 = alpha1 * a0;
    const float a2 = alpha2 * a0;

    float sample_n_2 = 0.0f;
    float sample_n_1 = 0.0f;
    for (size_t index = 0, count = input.size(); index < count; ++index)
    {
        float sample_n = input[index];
        output[index] = sample_n * a0 + sample_n_1 * a1 + sample_n_2 * a2;
        sample_n_2 = sample_n_1;
        sample_n_1 = sample_n;
    }
}

// spit out a CSV with details of an order 1 FIR filter:
//  * difference equation
//  * location of zero
//  * frequency and phase response
void ReportOrder1Filter(const char* fileName, float a0, float alpha1)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "wt");

    const float a1 = alpha1 * a0;

    fprintf(file, "\"Frequency\",\"Amplitude\",\"Phase\",");
    fprintf(file, "\"\",\"a0 = %f, alpha1 = %f\"\n", a0, alpha1);

    // calculate frequency & phase response
    std::vector<complex> response(c_numFrequencies);
    for (size_t index = 0; index < c_numFrequencies; ++index)
    {
        float percent = float(index) / float(c_numFrequencies - 1);
        float angle = percent * c_pi;

        complex response = a0 * (1.0f + alpha1 * Z(-1, angle));

        fprintf(file, "\"%f\",\"%f\",\"%f\"", percent, std::abs(response), atan2(response.imag(), response.real()));

        if (index == 1)
            fprintf(file, ",\"\",\"output[index] = input[index] * %f + input[index-1] * %f\"\n", a0, a1);
        else if (index == 3)
            fprintf(file, ",\"\",\"Zero = %f\"\n", -alpha1);
        else
            fprintf(file, "\n");
    }

    fclose(file);
}

// spit out a CSV with details of an order 2 FIR filter:
//  * difference equation
//  * location of zero
//  * frequency and phase response
void ReportOrder2Filter(const char* fileName, float a0, float alpha1, float alpha2)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "wt");

    const float a1 = alpha1 * a0;
    const float a2 = alpha2 * a0;

    // calculate the 2 zeroes
    complex zero1, zero2;
    {
        complex left, right;
        left.real(-alpha1 / 2.0f);

        float discriminant = alpha1 * alpha1 - 4.0f * alpha2;
        if (discriminant < 0.0f)
            right.imag(sqrt(-discriminant) / 2.0f);
        else
            right.imag(sqrt(discriminant) / 2.0f);

        zero1 = left - right;
        zero2 = left + right;
    }

    fprintf(file, "\"Frequency\",\"Amplitude\",\"Phase\",");
    fprintf(file, "\"\",\"a0 = %f, alpha1 = %f, alpha2 = %f\"\n", a0, alpha1, alpha2);

    // calculate frequency & phase response
    std::vector<complex> response(c_numFrequencies);
    for (size_t index = 0; index < c_numFrequencies; ++index)
    {
        float percent = float(index) / float(c_numFrequencies - 1);
        float angle = percent * c_pi;

        complex response = a0 * (1.0f + alpha1 * Z(-1, angle) + alpha2 * Z(-2, angle));

        fprintf(file, "\"%f\",\"%f\",\"%f\"", percent, std::abs(response), atan2(response.imag(), response.real()));

        if (index == 1)
            fprintf(file, ",\"\",\"output[index] = input[index] * %f + input[index-1] * %f + input[index-1] * %f\"\n", a0, a1, a2);
        else if (index == 3)
            fprintf(file, ",\"\",\"Zeroes = %f + %fi, %f + %fi\"\n", zero1.real(), zero1.imag(), zero2.real(), zero2.imag());
        else
            fprintf(file, "\n");
    }

    fclose(file);
}

int main(int argc, char**argv)
{
    // order 1 filters
    {
        // box filter low pass filter
        ReportOrder1Filter("1_lpf.csv", 0.5f, 1.0f);

        // a high pass filter in the same style
        ReportOrder1Filter("1_hpf.csv", 0.5f, -1.0f);

        // a weaker LPF that also is not linear phase
        ReportOrder1Filter("1_lpf2.csv", 0.5f, 2.0f);
    }

    // order 2 filters
    {
        // a low pass filter
        ReportOrder2Filter("2_lpf.csv", 0.5f, 2.0f, 1.22f);

        // a high pass filter
        ReportOrder2Filter("2_hpf.csv", 0.5f, -1.6f, 0.8f);

        // a notch filter at 1/2 nyquist
        ReportOrder2Filter("2_notch.csv", 0.5f, 0.0f, 1.0f);
    }

    return 0;
}