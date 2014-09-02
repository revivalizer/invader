#pragma once

namespace invader {

// Compile time sin/cos
template<unsigned M, unsigned N, unsigned B, unsigned A>
struct SinCosSeries {
	static double value() {
		return 1-(A*kM_PI/B)*(A*kM_PI/B)/M/(M+1)
		         *SinCosSeries<M+2,N,B,A>::value();
	}
};

template<unsigned N, unsigned B, unsigned A>
struct SinCosSeries<N,N,B,A> {
	static double value() { return 1.; }
};

template<unsigned B, unsigned A, typename T=double>
struct Sin;
 
template<unsigned B, unsigned A>
struct Sin<B,A,float> {
	static float value() {
		return (A*kM_PI/B)*SinCosSeries<2,24,B,A>::value();
	}
};

template<unsigned B, unsigned A>
struct Sin<B,A,double> {
	static double value() {
		return (A*kM_PI/B)*SinCosSeries<2,34,B,A>::value();
	}
};

template<unsigned B, unsigned A, typename T=double>
struct Cos;

template<unsigned B, unsigned A>
struct Cos<B,A,float> {
	static float value() {
		return SinCosSeries<1,23,B,A>::value();
	}
};

template<unsigned B, unsigned A>
struct Cos<B,A,double> {
	static double value() {
		return SinCosSeries<1,33,B,A>::value();
	}
};

namespace complex
{

	// General recursive function
	template<unsigned N, typename C=complex_t, typename T=double>
	class DanielsonLanczos {
		DanielsonLanczos<N/2,C,T> next;
	public:
		void apply(C* data) {
			next.apply(data);
			//next.apply(data+N);
			next.apply(data+N/2);

			//T wtemp,tempr,tempi,wr,wi,wpr,wpi;
			//T wtemp,tempr,tempi,wr,wi,wpr,wpi;
			T wtemp;
			wtemp = -Sin<N,1,T>::value();
			//wpr = -2.0*wtemp*wtemp;
			//wpi = -Sin<N,2,T>::value();
			//wr = 1.0;
			//wi = 0.0;
			auto wp = C(-2.0*wtemp*wtemp, -Sin<N,2,T>::value());
			auto w  = C(1.0, 0.0);
			//for (unsigned i=0; i<N; i+=2) {
			for (unsigned i=0; i<N/2; i++) {
				//tempr = data[i+N]*wr - data[i+N+1]*wi;
				//tempi = data[i+N]*wi + data[i+N+1]*wr;
				auto temp = data[i+N/2]*w;
				//data[i+N] = data[i]-tempr;
				//data[i+N+1] = data[i+1]-tempi;
				data[i+N/2] = data[i] - temp;
				//data[i] += tempr;
				//data[i+1] += tempi;
				data[i] += temp;

				//wtemp = wr;
				//wr += wr*wpr - wi*wpi;
				//wi += wi*wpr + wtemp*wpi;
				w += w*wp;
			}
		}
	};

	// Specializations for small N
	template<typename C, typename T>
	class DanielsonLanczos<4,C,T> {
	public:
		void apply(C* data) {
			//T tr = data[2];
			//T ti = data[3];
			auto t = data[1];
			//data[2] = data[0]-tr;
			//data[3] = data[1]-ti;
			data[1] = data[0]-t;
			//data[0] += tr;
			//data[1] += ti;
			data[0] += t;
			//tr = data[6];
			//ti = data[7];
			t = data[3];
			//data[6] = data[5]-ti;
			//data[7] = tr-data[4]; // i*(t-data[2])
			data[3] = C::i() * (t-data[2]);
			//data[4] += tr;
			//data[5] += ti;
			data[2] += t;

			//tr = data[4];
			//ti = data[5];
			t = data[2];
			//data[4] = data[0]-tr;
			//data[5] = data[1]-ti;
			data[2] = data[0]-t;
			//data[0] += tr;
			//data[1] += ti;
			data[0] += t;
			//tr = data[6];
			//ti = data[7];
			t = data[3];
			//data[6] = data[2]-tr;
			//data[7] = data[3]-ti;
			data[3] = data[1]-t;
			//data[2] += tr;
			//data[3] += ti;
			data[1] += t;
		}
	};
	 
	template<typename C, typename T>
	class DanielsonLanczos<2,C,T> {
	public:
		void apply(C* data) {
			//T tr = data[2];
			//T ti = data[3];
			auto t = data[1];
			//data[2] = data[0]-tr;
			//data[3] = data[1]-ti;
			data[1] = data[0]-t;
			//data[0] += tr;
			//data[1] += ti;
			data[0] += t;
		}
	};

	uint32_t bitrev(uint32_t x, uint32_t numBits);

	template<typename C>
	void swap(C& a, C& b)
	{
		C t = a;
		a = b;
		b = t;
	};

	template<typename C>
	void scramble(C* data, uint32_t N, uint32_t numBits)
	{
		numBits;
		/*
		for (uint32_t i=0; i<N; i++)
		{
			uint32_t j = bitrev(i, numBits);
			
			if (i < j)
				swap(data[i], data[j]);
		}*/
	   //void apply(Complex<T>* data) {
	     unsigned long m,j=0;
	     for (unsigned long i=0; i<N; ++i) {
	        if (j>i) {
	            swap(data[j], data[i]);
	        }
	        m = N/2;
	        while (m>=1 && j>=m) {
	            j -= m;
	            m >>= 1;
	        }
	        j += m;
	     }
	   //}
	}

	template<unsigned P,
	         typename C=complex_t,
	         typename T=double>
	class GFFT {
		enum { N = 1<<P };
		DanielsonLanczos<N,C,T> recursion;
	public:
		void fft(C* data) {
			// Normalize
			/*for (uint32_t i=0; i<N; i++)
			{
				data[i*2+0] *= 1.0/T(N);
				data[i*2+1] *= 1.0/T(N);
			}*/
			// Switch to scalar multiplication
			C inverse(1.0/double(N));
			for (uint32_t i=0; i<N; i++)
				data[i] *= inverse;

			scramble(data, N, P);
			recursion.apply(data);
		}

		void ifft(C* data) {
			// Conjugate
			for (uint32_t i=0; i<N; i++)
				//data[i*2+1] = -data[i*2+1];
				data[i] = -data[i];

			scramble(data, N, P);
			recursion.apply(data);

			// Conjugate
			for (uint32_t i=0; i<N; i++)
				//data[i*2+1] = -data[i*2+1];
				data[i] = -data[i];
		}

		void realfft(T* in, C* out)
		{
			// Interleave even and odd values
			for (uint32_t i=0; i<N; i++)
			{
				out[i] = C(in[i*2], in[i*2+1]);
			}

			fft(out);

			complex_t z0 = out[0];
			complex_t z0_ = 0.5*((z0+(-z0)) - complex_t::i()*(z0-(-z0)));
			complex_t zN_ = 0.5*((z0+(-z0)) + complex_t::i()*(z0-(-z0)));

			out[0] = complex_t(re(z0_), re(zN_));

			// Combine DIT butterfly
			for (uint32_t i=1; i<=N/2; i++)
			{
				// Combine both elements 
				complex_t z1 = out[i];
				complex_t z2 = out[N - i];

				double phase1 = -kM_PI2*i/(N*2.0);
				complex_t ce1 = complex_t(zcosd(phase1), zsind(phase1));

				complex_t z1_ = 0.5*((z1+(-z2)) - complex_t::i()*ce1*(z1-(-z2)));

				double phase2 = -kM_PI2*(N-i)/(N*2.0);
				complex_t ce2 = complex_t(zcosd(phase2), zsind(phase2));

				complex_t z2_ = 0.5*((z2+(-z1)) - complex_t::i()*ce2*(z2-(-z1)));

				out[i] = z1_;
				out[N-i] = z2_;
			}

		}

		void realifft(C* in, T* out)
		{
			/*// Interleave even and odd values
			for (uint32_t i=0; i<N; i++)
			{
				out[i] = C(in[i*2], in[i*2+1]);
			}*/

			// res
			//in[0]

			complex_t z0 = complex_t(re(in[0]));
			complex_t zN = complex_t(im(in[0]));

			im(in[0]) = 0.0;

			complex_t z0_ = 0.5*((z0+(-zN)) + complex_t::i()*(z0-(-zN)));

			in[0] = z0_;

			// Combine DIT butterfly
			for (uint32_t i=1; i<=N/2; i++)
			{
				// Combine both elements 
				complex_t z1 = in[i];
				complex_t z2 = in[N - i];

				double phase1 = +kM_PI2*i/(N*2.0);
				complex_t ce1 = complex_t(zcosd(phase1), zsind(phase1));

				complex_t z1_ = 0.5*((z1+(-z2)) + complex_t::i()*ce1*(z1-(-z2)));

				double phase2 = +kM_PI2*(N-i)/(N*2.0);
				complex_t ce2 = complex_t(zcosd(phase2), zsind(phase2));

				complex_t z2_ = 0.5*((z2+(-z1)) + complex_t::i()*ce2*(z2-(-z1)));

				in[i] = z1_;
				in[N-i] = z2_;
			}

			ifft(in);

			// Deinterleave values
			for (uint32_t i=0; i<N; i++)
			{
				out[i*2]   = re(in[i]);
				out[i*2+1] = im(in[i]);
			}

		}
	};

}

namespace real
{

	// General recursive function
	template<unsigned N, typename T=double>
	class DanielsonLanczos {
		DanielsonLanczos<N/2,T> next;
	public:
		void apply(T* data) {
			next.apply(data);
			next.apply(data+N);

			T wtemp,tempr,tempi,wr,wi,wpr,wpi;
			wtemp = -Sin<N,1,T>::value();
			wpr = -2.0*wtemp*wtemp;
			wpi = -Sin<N,2,T>::value();
			wr = 1.0;
			wi = 0.0;
			for (unsigned i=0; i<N; i+=2) {
				tempr = data[i+N]*wr - data[i+N+1]*wi;
				tempi = data[i+N]*wi + data[i+N+1]*wr;
				data[i+N] = data[i]-tempr;
				data[i+N+1] = data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;

				wtemp = wr;
				wr += wr*wpr - wi*wpi;
				wi += wi*wpr + wtemp*wpi;
			}
		}
	};

	// Specializations for small N
	template<typename T>
	class DanielsonLanczos<4,T> {
	public:
		void apply(T* data) {
			T tr = data[2];
			T ti = data[3];
			data[2] = data[0]-tr;
			data[3] = data[1]-ti;
			data[0] += tr;
			data[1] += ti;
			tr = data[6];
			ti = data[7];
			data[6] = data[5]-ti;
			data[7] = tr-data[4]; // subtraction followed by conjugation?
			data[4] += tr;
			data[5] += ti;

			tr = data[4];
			ti = data[5];
			data[4] = data[0]-tr;
			data[5] = data[1]-ti;
			data[0] += tr;
			data[1] += ti;
			tr = data[6];
			ti = data[7];
			data[6] = data[2]-tr;
			data[7] = data[3]-ti;
			data[2] += tr;
			data[3] += ti;
		}
	};
	 
	template<typename T>
	class DanielsonLanczos<2,T> {
	public:
		void apply(T* data) {
			T tr = data[2];
			T ti = data[3];
			data[2] = data[0]-tr;
			data[3] = data[1]-ti;
			data[0] += tr;
			data[1] += ti;
		}
	};

	template<typename T>
	void swap(T& a, T& b)
	{
		T t = a;
		a = b;
		b = t;
	}

	template<typename T>
	void scramble(T* data, uint32_t nn)
	{
		// This from Numerical Methods 'four1' C implementation
		uint32_t n, j, i, m;

		n = nn<<1;
		j=1;
		for (i=1; i<n; i+=2) {
			if (j>i) {
				swap(data[j-1], data[i-1]);
				swap(data[j], data[i]);
			}
			m = nn;
			while (m>=2 && j>m) {
				j -= m;
				m >>= 1;
			}
			j += m;
		};
	}

	template<unsigned P,
	         typename T=double>
	class GFFT {
		enum { N = 1<<P };
		DanielsonLanczos<N,T> recursion;
	public:
		void fft(T* data) {

			// Swap indices
			for (uint32_t i=0; i<N; i++)
				swap(data[i*2], data[i*2+1]);

			// Normalize, assuming real data
			for (uint32_t i=0; i<N; i++)
			{
				data[i*2+0] *= 1.0/T(N);
				data[i*2+1] *= 1.0/T(N);
			}

			scramble(data, N);
			recursion.apply(data);

			// Swap indices
			for (uint32_t i=0; i<N; i++)
				swap(data[i*2], data[i*2+1]);
		}

		void ifft(T* data) {
			// Swap indices
			for (uint32_t i=0; i<N; i++)
				swap(data[i*2], data[i*2+1]);

			// Conjugate
			for (uint32_t i=0; i<N; i++)
				data[i*2+1] = -data[i*2+1];

			scramble(data, N);
			recursion.apply(data);

			// Conjugate
			for (uint32_t i=0; i<N; i++)
				data[i*2+1] = -data[i*2+1];

			// Swap indices
			for (uint32_t i=0; i<N; i++)
				swap(data[i*2], data[i*2+1]);

		}

		void realfft(T* data)
		{
			// Interleave even and odd values
			for (uint32_t i=0; i<N; i++)
			{
			}

			fft(data);
		}
	};

}

} // namespace invader